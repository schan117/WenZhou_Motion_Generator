#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Motion_Controller.h"
#include "Status_Thread.h"
#include "Circular_Thread.h"
#include "Script_Reader.h"
#include <QProgressDialog>
#include "External_Thread.h"


#define CIRCULAR    0
#define EXTERNAL    1


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void Connect_Signals();

    bool Initialize_Motion_Table(QString filename);
    void Initialize_Status_Thread();
    void Initialize_Circular_Thread();
    void Initialize_External_Thread();
    bool Home_Table();
    bool Is_Table_Under_Alarm(int* is_under_alarm);
    bool Clear_Table_Status();
    bool Turn_On_All_Axis();

protected:

    void closeEvent(QCloseEvent* event);

public slots:

    void On_Goto_0_0();
    void On_Goto_100_0();
    void On_Goto_0_100();
    void On_Goto_100_100();

    void On_Start_Motion();
    void On_Stop_Motion();
    void On_Quit();
    void On_Read_External_File();

    void On_Update_Coordinates(double x, double y);

    void On_Pause_Motion();
    void On_Alarm_Triggered(int i);
    void On_Positive_Limit_Triggered(int i);
    void On_Negative_Limit_Triggered(int i);

private slots:
    void on_ToggleLed_stateChanged(int arg1);
    void On_Motion_Completed();


    void on_Diameter_valueChanged(double arg1);

    void on_Frequency_valueChanged(double arg1);

private:
    Ui::MainWindow *ui;


    Motion_Controller mc;
    Status_Thread st;
    Circular_Thread ct;
    External_Thread et;

    int axis_count;
    double fast_home_vel;
    double slow_home_vel;
    double synVelMax_MMPS;
    double synAccMax_CPMS;
    int led_pin;

    QProgressDialog* home_progress_dialog;

    void Disable_Manual_Controls();
    void Enable_Manual_Controls();

    Script_Reader sr;

    bool alarm_triggered;
    bool positive_triggered;
    bool negative_triggered;

};

#endif // MAINWINDOW_H
