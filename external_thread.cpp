#include "external_thread.h"

External_Thread::External_Thread()
{
    keep_motioning = true;
    pause = false;
}

void External_Thread::Initialize(Motion_Controller *mmc, Script_Reader *sr, double syn_Acc)
{
    mc = mmc;
    synAcc = syn_Acc;
    ssr = sr;
}


void External_Thread::run()
{
    long remaining;

    while (keep_motioning)
    {
        do
        {
            mc->mutex.lock();
            mc->Get_Remaining_Motion(1, 1, &remaining);
            mc->mutex.unlock();
            msleep(20);
            qDebug() << "Segments in external buffer:" << remaining << "with synAcc:" << synAcc;
        } while (remaining > 1);

        if (keep_motioning)
        {
            if (pause)
            {
                while (pause)
                {
                    msleep(50);
                }

                mc->mutex.lock();
                Next_Line();
                mc->Push_XY_To_Buffer(1, 1, ssr->script[current_line].params[1].toDouble(), ssr->script[current_line].params[2].toDouble(), ssr->script[current_line].params[3].toDouble(), 0, synAcc);
                Next_Line();
                mc->Push_XY_To_Buffer(1, 1, ssr->script[current_line].params[1].toDouble(), ssr->script[current_line].params[2].toDouble(), ssr->script[current_line].params[3].toDouble(), 0, synAcc);

                mc->Start_Crd(1, 1);
                mc->mutex.unlock();
            }
            else
            {
                // push 2 more lines to external buffer
                mc->mutex.lock();
                Next_Line();
                mc->Push_XY_To_Buffer(1, 1, ssr->script[current_line].params[1].toDouble(), ssr->script[current_line].params[2].toDouble(), ssr->script[current_line].params[3].toDouble(), 0, synAcc);
                Next_Line();
                mc->Push_XY_To_Buffer(1, 1, ssr->script[current_line].params[1].toDouble(), ssr->script[current_line].params[2].toDouble(), ssr->script[current_line].params[3].toDouble(), 0, synAcc);
                mc->mutex.unlock();
            }
        }


    }

    short motioning;

    do
    {
        mc->mutex.lock();
        mc->Get_Crd_Status(1, 1 ,&motioning);
        mc->mutex.unlock();
        msleep(50);
    }while (motioning != 0);


}

void External_Thread::Next_Line()
{
    if ((current_line + 1) == ssr->script.count())
    {
        current_line = 1; // do not return to line 0 as line 0 is the initial position, instead line 1 is the first target location
    }
    else
    {
        current_line++;
    }
}
