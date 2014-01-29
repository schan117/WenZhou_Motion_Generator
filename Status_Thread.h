#pragma once

#include <qthread.h>
#include "Motion_Controller.h"


class Status_Thread : public QThread
{

Q_OBJECT

public:

	Status_Thread(void);
	~Status_Thread(void);

    void Initialize(Motion_Controller* mmc);

	void run();

    Motion_Controller* mc;

	bool keep_going;


signals:

    void Update_Positions(double prf_x, double prf_y);


	void Positive_Limit_Triggered(int i);
	void Negative_Limit_Triggered(int i);
	void Alarm_Triggered(int i);

	




};

