#pragma once


#include <QtCore>
#include <qvector3d.h>
#include "gts.h"
#include <QMutex>

struct Axis_Properties
{
	long count_per_rev;
	double down_ratio;
	double home_offset; // in mm or in degrees
	double zero_pos_angle;
	int is_open_loop;
	double pitch;
	double home_search_position;
	double positive_limit;
	double negative_limit;
	double home_backup;
	int is_limit_enabled;
};


class Motion_Controller : public QObject
{

Q_OBJECT


public:

	Motion_Controller(void);
	~Motion_Controller(void);

	void Initialize_Motion_Controller(int axes);
	void Set_Axis_Properties(int axis, Axis_Properties ap);

	bool Open_and_Load_GTS(QString filename);
	bool Clear_Axis_Status(int axis);
	bool Turn_On_Off_Axis(int axis, bool turn_on);
	bool Zero_Axis(int axis);
	bool Is_Positive_Limit_Triggered(int axis, int* is_triggered);
	bool Is_Negative_Limit_Triggered(int axis, int* is_triggered);
	bool Is_Axis_Profiling(int axis, int* is_profiling);
	bool Is_Axis_Encoder_In_Range(int axis, int* is_in_pos);
	bool Is_Axis_In_Alarm_State(int axis, int* is_in_alarm);
	bool Get_Profile_Position(int axis, double* profile_position);
	bool Set_Profile_Position(int axis, double profile_position);
	bool Get_Encoder_Position(int axis, double* encoder_position);
	bool Set_Encoder_Position(int axis, double encoder_position);
	bool Sync_Profile_Encoder_Position(int axis, double set_position);
	bool Set_Soft_Limit(int axis, double positive_limit, double negative_limit);
	bool Abrupt_Stop_All_Axes();
	bool Smooth_Stop_All_Axes();
	bool Disable_Axis_Limit_Switches(int axis);
	bool Enable_Axis_Limit_Switches(int axis);
    bool Get_Axis_Status(int axis, long* status);


	// Axis Jog Mode
    bool Jog_Axis(int axis, double vel, double acc, double dec, double smooth);

	// Axis P2P Mode
	bool P2P_Axis(int axis, double position, double vel, double vel_start, double acc, double dec, double smooth);

	// Axis Pt Mode
	bool Pt_Axis(int axis, int mode, int memory_mode);
	bool Clear_Pt(int axis, short fifo);
	bool Get_Pt_Buffer_Space(int axis, short fifo, short* buffer_space);
	bool Push_Physical_to_Pt_Buffer(int axis, double position, double time, short fifo, short type, double reference); // time in sec
	bool Start_Pt(long mask, long option);


	bool Go_Axes(long mask);
	bool Go_All_Robot_Axes();

	// IO Function
	bool Set_GPO_State(int pin, short state);
	bool Get_GPI_State(int pin, int* state);

    // Interpolation functions
    bool Setup_Coordinate_System(int axis_count, double synvel_max, double synacc_max);
    bool Setup_Look_Ahead_Buffer(short crd, short fifo, double T, double synAccMax);
    bool Push_Circular_Motion_to_Buffer(short crd, short fifo, double xc, double yc, double x_end, double y_end, double synVel, double vel_end, double synAcc);
    bool Push_XY_To_Buffer(short crd, short fifo, double x, double y, double synVel, double vel_end, double synAcc);
    bool Transfer_Look_Ahead_To_Buffer(short crd, short fifo);
    bool Clear_Buffer(short crd, short fifo);
    bool Get_Crd_Space(short crd, short fifo, long* space);
    bool Get_Remaining_Motion(short crd, short fifo, long* remaining);
    bool Start_Crd(short crd, short fifo);
    bool Get_Crd_Status(short crd, short fifo, short* is_motioning);

	
//private:

	int axis_count;
	Axis_Properties* axis_properties;

    // for setting up coordinate motion such as XY Table
    TCrdPrm crd_prm;
    TCrdData crd_data[200];

    QMutex mutex;

public:

	double Count_2_Physical(int axis, double count);
	double Physical_2_Count(int axis, double angle);
	double DPS_2_CPMS(int axis, double dps);

	
	
};

