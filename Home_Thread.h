#pragma once

#include <qthread.h>
#include "qprogressdialog.h"
#include "Motion_Con.h"


class Home_Thread : public QThread
{

Q_OBJECT

public:
	Home_Thread(void);
	~Home_Thread(void);

	void Initialize(Delta_Robot* delta_robot);
	void run();

	Delta_Robot* dr;

signals:

	void Homing_Completed(bool ok);

public:



};

