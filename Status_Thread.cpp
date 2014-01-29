#include "Status_Thread.h"



Status_Thread::Status_Thread(void)
{

	keep_going = true;


	
}


Status_Thread::~Status_Thread(void)
{
}

void Status_Thread::run()
{

	int is_positive_triggered;
	int is_negative_triggered;
	int is_under_alarm;

    double x;
    double y;

	while (keep_going)
	{
		// first, update position information

        mc->mutex.lock();
        mc->Get_Profile_Position(0, &x);
        mc->Get_Profile_Position(1, &y);
        mc->mutex.unlock();

        emit Update_Positions(x, y);

        // Get axis status,
        mc->mutex.lock();

        for (int i=0; i<mc->axis_count; i++)
		{
            mc->Is_Positive_Limit_Triggered(i, &is_positive_triggered);

			if (is_positive_triggered)
			{
				emit Positive_Limit_Triggered(i);
			}

            mc->Is_Negative_Limit_Triggered(i, &is_negative_triggered);

			if (is_negative_triggered)
			{
				emit Negative_Limit_Triggered(i);
			}

            mc->Is_Axis_In_Alarm_State(i, &is_under_alarm);

			if (is_under_alarm)
			{
				emit Alarm_Triggered(i);
			}
		}

		/////////////////////////////////////////////////////////////////////////////////////////

        mc->mutex.unlock();

        msleep(70);
	}

    qDebug("Status thread ended!");

	keep_going = true;
}

void Status_Thread::Initialize(Motion_Controller *mmc)
{
    mc = mmc;
}







