#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtTest/QTest>
#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("Metron Motion Generator"));
    this->show();
    this->setEnabled(false);
    this->update();

    bool temp;

    alarm_triggered = false;
    positive_triggered = false;
    negative_triggered = false;

    temp =  Initialize_Motion_Table("MT_Config.cfg");

    qDebug() << "Initialize Motion Table:" << temp;

    if (temp)
    {

        // Setup up threads ///////////////////////////////////////////////////////////////////////////////////////
        Initialize_Status_Thread();
        Initialize_Circular_Thread();
        Initialize_External_Thread();


        home_progress_dialog = new QProgressDialog("", 0, 0, 100, this, Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
        home_progress_dialog->setWindowModality(Qt::WindowModal);
        home_progress_dialog->setWindowTitle(tr("Homing In Progress"));
        home_progress_dialog->setLabelText(tr("Motion Generator is currently homing..."));
        home_progress_dialog->setMinimumWidth(400);

        Connect_Signals();

        int alarm;

        Is_Table_Under_Alarm(&alarm);

        while (alarm)
        {
            QMessageBox::question(this, tr("Table Alarm"), tr("Error #1: At least one axis is under alarm state.\nCheck system is powered up properly and clear any alarms"), QMessageBox::Ok);
            Is_Table_Under_Alarm(&alarm);
            Clear_Table_Status();
        }

        // Turn on servo motor

        QMessageBox::warning(this, tr("Servo Power"), tr("Servo system will be powered up."), QMessageBox::Ok);

        if (Turn_On_All_Axis())
        {
            int result = QMessageBox::information(this, tr("Servo Power"), tr("Servo system is currently powered up\nPress OK to start homing."), QMessageBox::Ok, QMessageBox::Cancel);

            if (result == QMessageBox::Ok)
            {
                home_progress_dialog->show();
                home_progress_dialog->setValue(50);

                bool ok;

                ok = Home_Table();

                // check once more that no axes are under alarm state

                int alarm;

                Is_Table_Under_Alarm(&alarm);

                if (ok && !alarm)
                {
                    home_progress_dialog->setValue(100);


                    // homing is ok at this point
                    // Setup coordinate system for interpolation

                    bool temp;

                    temp = mc.Setup_Coordinate_System(axis_count, this->synVelMax_MMPS, this->synAccMax_CPMS);

                    qDebug() << "Setup Coordinate System:" << temp;

                    // Setup a look ahead buffer for fifo[1], fifo[0] will be used for circular buffer so no look ahead is needed while
                    // fifo[1] will be used for external data
                    //temp = mc.Setup_Look_Ahead_Buffer(1, EXTERNAL , 5, this->synAccMax_CPMS);

                    qDebug() << "Setup Look Ahead Buffer:" << temp;

                    // Clear buffers

                    temp = mc.Clear_Buffer(1, CIRCULAR);

                    qDebug() << "Clear Circular Buffer:" << temp;

                    temp = mc.Clear_Buffer(1, EXTERNAL);

                    qDebug() << "Clear External Buffer:" << temp;


                    // start status thread for coordinates display

                    st.start();
                    this->setEnabled(true);

                    Enable_Manual_Controls();

                }
                else
                {
                    QMessageBox::warning(this, tr("Homing"), tr("Error #2: Homing was not successful."), QMessageBox::Ok);

                }




            }

        }
        else
        {
            QMessageBox::information(this, tr("Servo Power"), tr("Error #3: At least one servo axis fails to power up."), QMessageBox::Ok);
        }
    }
    else
    {
        QMessageBox::warning(this, tr("Motion Controller"), tr("Error #8: Fail to open motion controller!"), QMessageBox::Ok);
    }


}

MainWindow::~MainWindow()
{


    // Turn off servo for all axes

    for(int i=0; i<axis_count; i++)
    {
        mc.Turn_On_Off_Axis(i, false);
    }

    if (st.isRunning())
    {
        st.keep_going = false;
    }

    while (st.isRunning() == true)
    {
        QTest::qWait(50);
    }


    delete home_progress_dialog;


    delete ui;
}

void MainWindow::Connect_Signals()
{
    connect(&st, SIGNAL(Update_Positions(double,double)), this, SLOT(On_Update_Coordinates(double,double)));
    connect(&ct, SIGNAL(finished()), this, SLOT(On_Motion_Completed()));
    connect(&et, SIGNAL(finished()), this, SLOT(On_Motion_Completed()));
    connect(&st, SIGNAL(Alarm_Triggered(int)), SLOT(On_Alarm_Triggered(int)));
    connect(&st, SIGNAL(Positive_Limit_Triggered(int)), this, SLOT(On_Positive_Limit_Triggered(int)));
    connect(&st, SIGNAL(Negative_Limit_Triggered(int)), this, SLOT(On_Negative_Limit_Triggered(int)));
}

bool MainWindow::Initialize_Motion_Table(QString filename)
{
    QSettings set("settings/Motion_Table.ini", QSettings::IniFormat);

    bool ok;

    axis_count = set.value("Controller/Axis_Count").toInt(&ok);
    if (!ok) return false;

    qDebug() << "Axis Count:" << axis_count;

    fast_home_vel = set.value("Controller/Fast_Home_Vel").toDouble(&ok);
    if (!ok) return false;

    qDebug() << "Fast Home Vel:" << fast_home_vel;

    slow_home_vel = set.value("Controller/Slow_Home_Vel").toDouble(&ok);
    if (!ok) return false;

    qDebug() << "Slow Home Vel:" << slow_home_vel;

    synVelMax_MMPS = set.value("Controller/SynVel_Max_MMPS").toDouble(&ok);
    if (!ok) return false;

    qDebug() << "SynVelMax MM/s:" << synVelMax_MMPS;

    synAccMax_CPMS = set.value("Controller/SynAcc_Max_CPMS").toDouble(&ok);
    if (!ok) return false;

    qDebug() << "SynAccMax Count/ms2:" <<  synAccMax_CPMS;

    led_pin = set.value("Output/Led").toInt(&ok);
    if (!ok) return false;

    qDebug() << "Led Pin:" << led_pin;



    mc.Initialize_Motion_Controller(axis_count);
    Axis_Properties ap;


    for (int i=0; i<axis_count; i++)
    {
        qDebug() << "Axis:" << i;

        ap.count_per_rev = set.value(QString("Axis%1/Count_Per_Rev").arg(i)).toInt(&ok);
        if (!ok)
        {
            return false;
        }
        qDebug() << "   Count Per Rev:" << ap.count_per_rev;

        ap.down_ratio = set.value(QString("Axis%1/Down_Ratio").arg(i)).toDouble(&ok);
        if (!ok)
        {
            return false;
        }
        qDebug() << "   Down Ratio:" << ap.down_ratio;

        ap.pitch = set.value(QString("Axis%1/Pitch").arg(i)).toDouble(&ok);
        if (!ok)
        {
            return false;
        }
        qDebug() << "   Pitch:" << ap.pitch;

        ap.home_offset = set.value(QString("Axis%1/Home_Offset").arg(i)).toDouble(&ok);
        if (!ok)
        {
            return false;
        }
        qDebug() << "   Home Offset:" << ap.home_offset;

        ap.zero_pos_angle = set.value(QString("Axis%1/Zero_Pos_Angle").arg(i)).toDouble(&ok);
        if (!ok)
        {
            return false;
        }
        qDebug() << "   Zero Pos Angle:" << ap.zero_pos_angle;

        ap.is_open_loop = set.value(QString("Axis%1/Is_Open_Loop").arg(i)).toInt(&ok);
        if (!ok)
        {
            return false;
        }
        qDebug() << "   Is Open Loop:" << ap.is_open_loop;

        ap.home_search_position = set.value(QString("Axis%1/Home_Search_Position").arg(i)).toDouble(&ok);
        if (!ok)
        {
            return false;
        }
        qDebug() << "   Home Search Position:"  << ap.home_search_position;

        ap.positive_limit = set.value(QString("Axis%1/Positive_Limit").arg(i)).toDouble(&ok);
        if (!ok)
        {
            return false;
        }
        qDebug() << "   Positive Limit:" << ap.positive_limit;

        ap.negative_limit = set.value(QString("Axis%1/Negative_Limit").arg(i)).toDouble(&ok);
        if (!ok)
        {
            return false;
        }
        qDebug() << "   Negative Limit:" << ap.negative_limit;

        ap.home_backup = set.value(QString("Axis%1/Home_Backup").arg(i)).toDouble(&ok);
        if (!ok)
        {
            return false;
        }
        qDebug() << "   Home Backup:" << ap.home_backup;


        ap.is_limit_enabled = set.value(QString("Axis%1/Limit_Enabled").arg(i)).toInt(&ok);
        if (!ok)
        {
            return false;
        }
        qDebug() << "   Is Limit Enabled:" << ap.is_limit_enabled;

        mc.Set_Axis_Properties(i, ap);

    }

    // open motion control card after all settings have been loaded
    ok = mc.Open_and_Load_GTS(filename);
    qDebug() << "Open and Load GTS:" << ok;
    if (!ok) return false;

     // Clear axis status
    for (int i=0; i<axis_count; i++)
    {
        ok = mc.Clear_Axis_Status(i);
    }

    return true;
}

void MainWindow::Initialize_Status_Thread()
{
    st.Initialize(&mc);
}

void MainWindow::Initialize_Circular_Thread()
{
    ct.Initialize(&mc, ui->Diameter, ui->Frequency, this->synAccMax_CPMS );
}

void MainWindow::Initialize_External_Thread()
{
    et.Initialize(&mc, &sr, this->synAccMax_CPMS);
}

bool MainWindow::Home_Table()
{
    bool ok;

    for (int i=0; i<axis_count; i++)
    {
        ok = mc.P2P_Axis(i, mc.axis_properties[i].home_search_position, fast_home_vel, 0, 1, 1, 25);
        if (!ok)return false;
    }

    // Start all 2 axes

    ok = mc.Go_Axes(3);
    if (!ok)return false;

    int is_limit0 = 1;
    int is_limit1 = 1;

    int is_motion0 = 0;
    int is_motion1 = 0;

    do
    {
        ok = mc.Is_Negative_Limit_Triggered(0, &is_limit0);
        if (!ok)return false;


        ok = mc.Is_Negative_Limit_Triggered(1, &is_limit1);
        if (!ok)return false;

        QTest::qWait(20);

    } while ( (is_limit0 != 1) || (is_limit1 != 1) );

    // Block unit all axis has stopped

    do
    {
        ok = mc.Is_Axis_Profiling(0, &is_motion0);
        if (!ok)return false;


        ok = mc.Is_Axis_Profiling(1, &is_motion1);
        if (!ok)return false;

        QTest::qWait(20);

    } while ( (is_motion0 != 0) || (is_motion1 != 0) );

    QTest::qWait(200);

    // All two axes triggered the positive limit sensor by now, backup a little bit
    // Zero all two axis first

    for (int i=0; i<axis_count; i++)
    {
        ok = mc.Zero_Axis(i);
        if (!ok) return false;
        ok = mc.P2P_Axis(i, mc.axis_properties[i].home_backup, fast_home_vel, 0, 1, 1, 25);
        if (!ok) return false;
    }

    // Start all axes again
    ok = mc.Go_Axes(3);
    if (!ok) return false;

    do
    {
        ok = mc.Is_Axis_Profiling(0, &is_motion0);
        if (!ok)return false;

        ok = mc.Is_Axis_Profiling(1, &is_motion1);
        if (!ok)return false;

        QTest::qWait(20);

    } while ( (is_motion0 != 0) || (is_motion1 != 0) );

     // All two axes should have been moved away from limit sensor by now, clear all status

    for (int i=0; i<axis_count; i++)
    {
        ok = mc.Clear_Axis_Status(i);
        if (!ok) return false;
        ok = mc.Zero_Axis(i);
        if (!ok) return false;
    }

    QTest::qWait(500);

    // Set Soft Limit

    for (int i=0; i<axis_count; i++)
    {
        ok = mc.Zero_Axis(i);
        if (!ok) return false;
        ok = mc.Set_Soft_Limit(i, mc.axis_properties[i].positive_limit, mc.axis_properties[i].negative_limit);
        if (!ok) return false;
    }

    return true;
}

bool MainWindow::Is_Table_Under_Alarm(int *is_under_alarm)
{
    bool ok;
    int p;
    int result = 0;

    for (int i=0; i<axis_count; i++)
    {
        ok = mc.Is_Axis_In_Alarm_State(i, &p);
        if (!ok) return false;

        result = result || p;

    }

    *is_under_alarm = result;

    return true;
}

bool MainWindow::Clear_Table_Status()
{
    bool ok;

    for (int i=0; i<axis_count; i++)
    {
        ok = mc.Clear_Axis_Status(i);
        if (!ok) return false;
    }

    return true;

}

bool MainWindow::Turn_On_All_Axis()
{
    bool ok;

    for (int i=0; i<axis_count; i++)
    {
        ok = mc.Turn_On_Off_Axis(i, true);
        if (!ok) return false;
    }

    return true;

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (ct.isRunning() || et.isRunning())
    {
         QMessageBox::warning(this, tr("Quit"), tr("Stop motion program before quitting."), QMessageBox::Ok);
         event->ignore();
         return;
    }

    int ret =  QMessageBox::question(this, tr("Quit"), tr("Are you sure to quit program?"), QMessageBox::Ok, QMessageBox::Cancel);

    if (ret == QMessageBox::Ok)
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }

}

void MainWindow::On_Goto_0_0()
{
    bool ok;



    mc.mutex.lock();

    ok = mc.Clear_Buffer(1, CIRCULAR);

    ok = mc.Push_XY_To_Buffer(1, CIRCULAR, 0, 0, 30, 0, this->synAccMax_CPMS);

    ok = mc.Start_Crd(1, CIRCULAR);

    mc.mutex.unlock();

    this->setEnabled(false);

    short motioning;

    do
    {
        mc.mutex.lock();
        mc.Get_Crd_Status(1, CIRCULAR, &motioning);
        mc.mutex.unlock();

        QTest::qWait(10);
    } while (motioning != 0);

    this->setEnabled(true);

}

void MainWindow::On_Goto_100_0()
{
    bool ok;

    mc.mutex.lock();

    ok = mc.Clear_Buffer(1, CIRCULAR);

    ok = mc.Push_XY_To_Buffer(1, CIRCULAR, 100, 0, 30, 0, this->synAccMax_CPMS);

    ok = mc.Start_Crd(1, CIRCULAR);

    mc.mutex.unlock();

    this->setEnabled(false);

    short motioning;

    do
    {
        mc.mutex.lock();
        mc.Get_Crd_Status(1, CIRCULAR, &motioning);
        mc.mutex.unlock();

        QTest::qWait(10);
    } while (motioning != 0);

    this->setEnabled(true);

}

void MainWindow::On_Goto_0_100()
{
    bool ok;

    mc.mutex.lock();

    ok = mc.Clear_Buffer(1, CIRCULAR);

    ok = mc.Push_XY_To_Buffer(1, CIRCULAR, 0, 100, 30, 0, this->synAccMax_CPMS);

    ok = mc.Start_Crd(1, CIRCULAR);

    mc.mutex.unlock();

    this->setEnabled(false);

    short motioning;

    do
    {
        mc.mutex.lock();
        mc.Get_Crd_Status(1, CIRCULAR, &motioning);
        mc.mutex.unlock();

        QTest::qWait(10);
    } while (motioning != 0);

    this->setEnabled(true);
}

void MainWindow::On_Goto_100_100()
{
    bool ok;

    mc.mutex.lock();

    ok = mc.Clear_Buffer(1, CIRCULAR);

    ok = mc.Push_XY_To_Buffer(1, CIRCULAR, 100, 100, 30, 0, this->synAccMax_CPMS);

    ok = mc.Start_Crd(1, CIRCULAR);

    mc.mutex.unlock();

    this->setEnabled(false);

    short motioning;

    do
    {
        mc.mutex.lock();
        mc.Get_Crd_Status(1, CIRCULAR, &motioning);
        mc.mutex.unlock();

        QTest::qWait(10);
    } while (motioning != 0);

    this->setEnabled(true);
}

void MainWindow::On_Start_Motion()
{
    if (ui->UseExternalData->isChecked() == false)
    {
        // Disable manual controls

        Disable_Manual_Controls();


        // Move to initial position before starting to circle

        QMessageBox::information(this, tr("Begin Motion"), tr("Press OK to move to initial position."), QMessageBox::Ok);

        bool ok;

        mc.mutex.lock();
            ok = mc.Clear_Buffer(1, CIRCULAR);
            ok = mc.Push_XY_To_Buffer(1, CIRCULAR, 0, 100, 30, 0, this->synAccMax_CPMS);
            ok = mc.Start_Crd(1, CIRCULAR);
        mc.mutex.unlock();

        short motioning;

        do
        {
            mc.mutex.lock();
            mc.Get_Crd_Status(1, CIRCULAR, &motioning);
            mc.mutex.unlock();
            QTest::qWait(20);
        } while (motioning != 0);

        // Moved to initial position, first push 2 circular buffers in

        QTest::qWait(200);

        mc.mutex.lock();
        mc.Clear_Buffer(1, CIRCULAR);

        double cycle_time = 1.0 / ui->Frequency->value();
        double synVel = 3.14159 * ui->Diameter->value() / cycle_time;

        mc.Push_Circular_Motion_to_Buffer(1, CIRCULAR, ui->Diameter->value()/2, 0, 0, 100, synVel, synVel, this->synAccMax_CPMS);
        mc.Push_Circular_Motion_to_Buffer(1, CIRCULAR, ui->Diameter->value()/2, 0, 0, 100, synVel, synVel, this->synAccMax_CPMS);

        int ret = QMessageBox::question(this ,tr("Begin Motion"), tr("Press OK to begin circular motion."), QMessageBox::Ok, QMessageBox::Cancel);

        if (ret == QMessageBox::Ok)
        {
            ct.keep_circling = true;
            ct.start();

            // Start buffer!
            mc.Start_Crd(1, CIRCULAR);
            mc.mutex.unlock();
        }
        else
        {
            // not starting motion
            mc.mutex.unlock();
            Enable_Manual_Controls();
        }

     }
    else
    {
        // do external motion here
        // Disable manual controls

        if (sr.script.count() == 0)
        {
             QMessageBox::information(this, tr("Begin Motion"), tr("Load in script first!"), QMessageBox::Ok);
            return;
        }

        Disable_Manual_Controls();

        // Move to initial position before starting to circle
        double ix = sr.script[0].params[1].toDouble();
        double iy = sr.script[0].params[2].toDouble();
        QMessageBox::information(this, tr("Begin Motion"), QString(tr("Using external data. Press OK to move to initial position: [%1, %2]")).arg(ix).arg(iy), QMessageBox::Ok);

        bool ok;

        mc.mutex.lock();
            ok = mc.Clear_Buffer(1, EXTERNAL);
            ok = mc.Push_XY_To_Buffer(1, EXTERNAL, ix, iy, 30, 0, this->synAccMax_CPMS);
            ok = mc.Start_Crd(1, EXTERNAL);
        mc.mutex.unlock();

        short motioning;

        do
        {
            mc.mutex.lock();
            mc.Get_Crd_Status(1, EXTERNAL, &motioning);
            mc.mutex.unlock();
            QTest::qWait(20);
        } while (motioning != 0);

        // Moved to initial position, first push 2 lines

        QTest::qWait(200);

        mc.mutex.lock();
        mc.Clear_Buffer(1, EXTERNAL);

        // push first 2 lines of script to buffer

        mc.Push_XY_To_Buffer(1, EXTERNAL, sr.script[1].params[1].toDouble(), sr.script[1].params[2].toDouble(), sr.script[1].params[3].toDouble(), 0, this->synAccMax_CPMS);
        mc.Push_XY_To_Buffer(1, EXTERNAL, sr.script[2].params[1].toDouble(), sr.script[2].params[2].toDouble(), sr.script[2].params[3].toDouble(), 0, this->synAccMax_CPMS);
        et.current_line = 2;

        int ret = QMessageBox::question(this ,tr("Begin Motion"), tr("Press OK to begin external motion."), QMessageBox::Ok, QMessageBox::Cancel);

        if (ret == QMessageBox::Ok)
        {
            et.keep_motioning = true;
            et.start();

            // Start buffer!
            mc.Start_Crd(1, EXTERNAL);
            mc.mutex.unlock();
        }
        else
        {
            // not starting motion
            Enable_Manual_Controls();
            mc.mutex.unlock();
        }
    }



}

void MainWindow::On_Stop_Motion()
{
   int ret = QMessageBox::question(this, tr("Stop Motion"), tr("Are you sure to stop current motion program?"), QMessageBox::Ok, QMessageBox::Cancel);

   if (ret == QMessageBox::Ok)
   {
       if(ct.isRunning())
       {
            ct.keep_circling = false;
       }
       else if (et.isRunning())
       {
            et.keep_motioning = false;
       }
   }
}

void MainWindow::On_Quit()
{
    this->close();
}

void MainWindow::On_Read_External_File()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select Script"), 0, "Script (*.txt)");

    if (sr.Read_Script_File(filename))
    {
        int error_line = -1;

        int ret = sr.Check_Internal_Script_XY_Table(3, 0, -1, 201, -1, 201, synVelMax_MMPS, &error_line );

        qDebug() << "Script check result:" <<   ret;
        qDebug() << "First error in line:" << error_line;

        switch (ret)
        {
        case ERR_OK:
        {
            QMessageBox::information(this, tr("Load Script"), tr("Script loaded!"), QMessageBox::Ok);
            break;
        }
        case ERR_LINE_COUNT:
        {
            QMessageBox::warning(this, tr("Load Script"), tr("Error #4: At least 3 lines of scripts are required!"), QMessageBox::Ok);
            sr.Clear_Internal_Script();
            break;
        }
        case ERR_FIRST_LAST_NOT_EQUAL:
        {
            QMessageBox::warning(this, tr("Load Script"), tr("Error #5: Coordinates of first and last line of script are not equal!"), QMessageBox::Ok);
            sr.Clear_Internal_Script();
            break;
        }
        case ERR_OUT_OF_RANGE:
        {
            QMessageBox::warning(this, tr("Load Script"), QString(tr("Error #6\nLine %1: Parameters out of range!").arg(error_line)), QMessageBox::Ok);
            sr.Clear_Internal_Script();
            break;
        }
        case ERR_SYNTAX:
        {
            QMessageBox::warning(this, tr("Load Script"), QString(tr("Error #7\nLine %1: Syntax error!").arg(error_line)), QMessageBox::Ok);
            sr.Clear_Internal_Script();
            break;
        }
        }
    }
    else
    {
        QMessageBox::information(this, tr("Load Script"), tr("Scripted was NOT loaded properly"), QMessageBox::Ok);
    }
}

void MainWindow::On_Update_Coordinates(double x, double y)
{
    ui->X->setText(QString("%1").arg(x, 0, 'f', 3));
    ui->Y->setText(QString("%1").arg(y, 0, 'f', 3));
}

void MainWindow::On_Pause_Motion()
{
    if (ct.isRunning())
    {
        if (ui->actionPause->isChecked())
        {
            ct.pause = true;
        }
        else
        {
            ct.pause = false;
        }
    }
    if (et.isRunning())
    {
        if (ui->actionPause->isChecked())
        {
            et.pause = true;
        }
        else
        {
            et.pause = false;
        }
    }
}

void MainWindow::On_Alarm_Triggered(int i)
{
    if (alarm_triggered == false)
    {
         QMessageBox::critical(this, tr("Alarm"), QString(tr("Error #9\nAlarm triggered during motion!")), QMessageBox::Ok);
         alarm_triggered = true;

         // there was an alarm state triggered, write down status for each status and check whats going on

         QFile qf("Errorlog.txt");

         qf.open(QFile::Append | QFile::Text);

         QTextStream qts(&qf);

         QDateTime time = QDateTime::currentDateTime();
         QString time_string = time.toString("dd.MM.yyyy hh:mm:ss:zzz");

         qts << time_string << " ";

         long status = 0;
         double position = 0;
         double encoder = 0;

         for (int i=0; i<axis_count; i++)
         {
            mc.Get_Axis_Status(i, &status);
            mc.Get_Profile_Position(i, &position );
            mc.Get_Encoder_Position(i, &encoder );
            qts << QString("Axis%1 ").arg(i) << status << " " << "Position " << position << " " << "Encoder " << encoder;
         }

         qts << "\n\r";

         qf.close();
    }
}

void MainWindow::On_Positive_Limit_Triggered(int i)
{
    if (positive_triggered == false)
    {

            qDebug() << "Positive Triggered:" << i;
            positive_triggered = true;

    }
}

void MainWindow::On_Negative_Limit_Triggered(int i)
{
    if (negative_triggered == false)
    {

            qDebug() << "Negative Triggered:" << i;
            negative_triggered = true;

    }

}

void MainWindow::on_ToggleLed_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        mc.Set_GPO_State(led_pin, 0);
    }
    else
    {
        mc.Set_GPO_State(led_pin, 1);
    }
}

void MainWindow::On_Motion_Completed()
{
    Enable_Manual_Controls();
}

void MainWindow::Disable_Manual_Controls()
{
    ui->actionStart_Motion->setEnabled(false);
    ui->actionStop_Motion->setEnabled(true);
    ui->actionRead_External_File->setEnabled(false);
    ui->actionQuit->setEnabled(false);
    ui->groupBox->setEnabled(false);
    ui->UseExternalData->setEnabled(false);
    ui->actionPause->setEnabled(true);
}

void MainWindow::Enable_Manual_Controls()
{
    ui->actionStart_Motion->setEnabled(true);
    ui->actionPause->setEnabled(false);
    ui->actionStop_Motion->setEnabled(false);
    ui->actionRead_External_File->setEnabled(true);
    ui->actionQuit->setEnabled(true);
    ui->groupBox->setEnabled(true);
    ui->UseExternalData->setEnabled(true);
}




void MainWindow::on_Diameter_valueChanged(double arg1)
{
    // if diameter is greater than 100 mm while frequency is greater than 1 Hz, make sure diameter is going to be less than 100
    mc.mutex.lock();
    if (arg1 > 100)
    {
        if (ui->Frequency->value() > 1)
        {
            disconnect(ui->Diameter, SIGNAL(valueChanged(double)), this, SLOT(on_Diameter_valueChanged(double)));
            ui->Diameter->setValue(100.0);
            connect(ui->Diameter, SIGNAL(valueChanged(double)), this, SLOT(on_Diameter_valueChanged(double)));
        }
    }
    mc.mutex.unlock();

}

void MainWindow::on_Frequency_valueChanged(double arg1)
{
    mc.mutex.lock();
    if (arg1 > 1)
    {
        if (ui->Diameter->value() > 100)
        {
            disconnect(ui->Frequency, SIGNAL(valueChanged(double)), this, SLOT(on_Frequency_valueChanged(double)));
            ui->Frequency->setValue(1.0);
            connect(ui->Frequency, SIGNAL(valueChanged(double)), this, SLOT(on_Frequency_valueChanged(double)));
        }
    }
    mc.mutex.unlock();

}
