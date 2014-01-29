#include "circular_thread.h"
#include <QtTest>

Circular_Thread::Circular_Thread()
{
    keep_circling = true;
    pause = false;
}

void Circular_Thread::Initialize(Motion_Controller *mmc, QDoubleSpinBox* diameter, QDoubleSpinBox* frequency, double syn_Acc)
{
    mc = mmc;
    dia = diameter;
    freq = frequency;
    synAcc = syn_Acc;
}

void Circular_Thread::run()
{
    long remaining;

    while (keep_circling)
    {
       // block until there is only one segment left



        do
        {
            mc->mutex.lock();
            mc->Get_Remaining_Motion(1, 0, &remaining);
            mc->mutex.unlock();
            msleep(20);
            qDebug() << "Segments in circular buffer:" << remaining << " with synAcc:" << synAcc;
        } while (remaining > 1);


        double cycle_time = 1.0 / freq->value();
        double synVel = 3.14159 * dia->value() / cycle_time;

        if (keep_circling)
        {
            if (pause)
            {
                // If pause, push a smooth stop circle into buffer to make stopping nicer

                mc->mutex.lock();
                mc->Push_Circular_Motion_to_Buffer(1, 0, dia->value()/2, 0, 0, 100, synVel, 0, synAcc);
                mc->mutex.unlock();

                while (pause)
                {
                    msleep(50);
                }

                // push 2 more circles
                mc->mutex.lock();
                mc->Push_Circular_Motion_to_Buffer(1, 0, dia->value()/2, 0, 0, 100, synVel, synVel, synAcc);
                mc->Push_Circular_Motion_to_Buffer(1, 0, dia->value()/2, 0, 0, 100, synVel, synVel, synAcc);

                mc->Start_Crd(1, 0);
                mc->mutex.unlock();




            }
            else
            {
                // push 2 more circles
                mc->mutex.lock();
                mc->Push_Circular_Motion_to_Buffer(1, 0, dia->value()/2, 0, 0, 100, synVel, synVel, synAcc);
                mc->Push_Circular_Motion_to_Buffer(1, 0, dia->value()/2, 0, 0, 100, synVel, synVel, synAcc);
                mc->mutex.unlock();
             }
        }
        else
        {
            // push 1 more circle for a smooth stop
            mc->mutex.lock();
            mc->Push_Circular_Motion_to_Buffer(1, 0, dia->value()/2, 0, 0, 100, synVel, 0, synAcc);
            mc->mutex.unlock();
        }

    }

    // block until motion has been completed before quitting

    short motioning;

    do
    {
        mc->mutex.lock();
        mc->Get_Crd_Status(1, 0 ,&motioning);
        mc->mutex.unlock();
        msleep(50);
    }while (motioning != 0);








}
