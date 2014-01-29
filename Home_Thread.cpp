#include "StdAfx.h"
#include "Home_Thread.h"
#include "moc_Home_Thread.cpp"


Home_Thread::Home_Thread(void)
{
	

}


Home_Thread::~Home_Thread(void)
{

}

void Home_Thread::Initialize(Delta_Robot* delta_robot)
{
	dr = delta_robot;
	

}

void Home_Thread::run()
{
	bool ok = dr->Home_Delta_Robot();	

	emit Homing_Completed(ok);

}
