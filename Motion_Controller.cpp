#include "Motion_Controller.h"


Motion_Controller::Motion_Controller(void)
{
	axis_properties = NULL;
}


Motion_Controller::~Motion_Controller(void)
{
	if (axis_properties != NULL)
	{
		delete [] axis_properties;
	}

	short rtn = GT_Close();
	
}

void Motion_Controller::Initialize_Motion_Controller(int axes)
{
	axis_count = axes;
	axis_properties = new Axis_Properties[axes];
}

bool Motion_Controller::Open_and_Load_GTS(QString filename)
{
	// Open motion controller
	short rtn = GT_Open();

	if (rtn != 0)
	{
		return false;
	}

	// Perform a reset first
	rtn = GT_Reset();

	if (rtn != 0)
	{
		return false;
	}

	// Load configuration file

    rtn = GT_LoadConfig(filename.toLatin1().data());

	if (rtn != 0)
	{
		return false;
	}

	return true;
	
}

bool Motion_Controller::Clear_Axis_Status(int axis)
{
	short rtn = GT_ClrSts(axis+1);

	if (rtn != 0)
	{
		return false;
	}

	long Sts;
	
	rtn = GT_GetSts(axis+1, &Sts);

	if (rtn != 0)
	{
		return false;
	}

	// Check if axis is still in alarm state

	if ((Sts & 2) == 2)
	{
		// i.e, in alarm state
		return false;
	}
	else
	{
		return true;
	}
}

bool Motion_Controller::Turn_On_Off_Axis(int axis, bool turn_on)
{
	short rtn;
	
	if (turn_on)
	{
		rtn = GT_AxisOn(axis+1);
	}
	else
	{
		rtn = GT_AxisOff(axis+1);
	}
	
	if (rtn != 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void Motion_Controller::Set_Axis_Properties(int axis, Axis_Properties ap)
{
	axis_properties[axis].count_per_rev = ap.count_per_rev;
	axis_properties[axis].down_ratio = ap.down_ratio;
	axis_properties[axis].is_open_loop = ap.is_open_loop;
	axis_properties[axis].home_offset = ap.home_offset;
	axis_properties[axis].pitch = ap.pitch;
	axis_properties[axis].home_search_position = ap.home_search_position;
	axis_properties[axis].positive_limit = ap.positive_limit;
	axis_properties[axis].negative_limit = ap.negative_limit;
	axis_properties[axis].home_backup = ap.home_backup;
	axis_properties[axis].zero_pos_angle = ap.zero_pos_angle;
	axis_properties[axis].is_limit_enabled = ap.is_limit_enabled;


}

bool Motion_Controller::Zero_Axis(int axis)
{
	short rtn = GT_ZeroPos(axis+1);

	if (rtn != 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool Motion_Controller::Is_Positive_Limit_Triggered(int axis, int* is_triggered)
{
	long Sts;

	short rtn = GT_GetSts(axis+1, &Sts);

	if (rtn != 0)
	{
		return false;
	}

	if ((Sts & 32) == 32)
	{
		*is_triggered = 1;
	}
	else
	{
		*is_triggered = 0;
	}

	return true;
}

bool Motion_Controller::Is_Negative_Limit_Triggered(int axis, int* is_triggered)
{
	long Sts;

	short rtn = GT_GetSts(axis+1, &Sts);

	if (rtn != 0)
	{
		return false;
	}

	if ((Sts & 64) == 64)
	{
		*is_triggered = 1;
	}
	else
	{
		*is_triggered = 0;
	}

	return true;
}

bool Motion_Controller::Is_Axis_Profiling(int axis, int* is_profiling)
{
	long Sts;

	short rtn = GT_GetSts(axis+1, &Sts);

	if (rtn != 0)
	{
		return false;
	}

	if ((Sts & 1024) == 1024)
	{
		*is_profiling = 1;
	}
	else
	{
		*is_profiling = 0;
	}

	return true;

}

bool Motion_Controller::Is_Axis_Encoder_In_Range(int axis, int* is_in_pos)
{
	long Sts;

	short rtn = GT_GetSts(axis+1, &Sts);

	if (rtn != 0)
	{
		return false;
	}

	if ((Sts & 2048) == 2048)
	{
		*is_in_pos = 1;
	}
	else
	{
		*is_in_pos = 0;
	}

	return true;

	
}

bool Motion_Controller::Is_Axis_In_Alarm_State(int axis, int* is_in_alarm)
{
	long Sts;

	short rtn = GT_GetSts(axis+1, &Sts);

	if (rtn != 0)
	{
		return false;
	}


	if ((Sts & 2) == 2)
	{
		*is_in_alarm = 1;
	}
	else
	{
		*is_in_alarm = 0;
	}

	return true;
}

double Motion_Controller::Count_2_Physical(int axis, double count)
{
	return count / axis_properties[axis].count_per_rev / axis_properties[axis].down_ratio * axis_properties[axis].pitch;
}

double Motion_Controller::Physical_2_Count(int axis, double angle)
{
	return angle / axis_properties[axis].pitch * axis_properties[axis].count_per_rev * axis_properties[axis].down_ratio;
}

double Motion_Controller::DPS_2_CPMS(int axis, double dps)
{
	return dps / axis_properties[axis].pitch * axis_properties[axis].count_per_rev * axis_properties[axis].down_ratio / 1000;
}

bool Motion_Controller::Get_Profile_Position(int axis, double* profile_position)
{
	double value;

	short rtn = GT_GetPrfPos(axis+1, &value);

	if (rtn != 0)
	{
		return false;
	}

	*profile_position = Count_2_Physical(axis, value);

	return true;
}

bool Motion_Controller::Get_Encoder_Position(int axis, double* encoder_position)
{
	double value;

	short rtn = GT_GetEncPos(axis+1, &value);

	if (rtn != 0)
	{
		return false;
	}

	*encoder_position = Count_2_Physical(axis, value);

	return true;

}

bool Motion_Controller::Set_Profile_Position(int axis, double profile_position)
{
	short rtn = GT_SetPrfPos(axis+1, (long) floor((Physical_2_Count(axis, profile_position)+0.5)));

	if (rtn != 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool Motion_Controller::Set_Encoder_Position(int axis, double encoder_position)
{
	short rtn = GT_SetEncPos(axis+1, (long) floor(Physical_2_Count(axis, encoder_position)+0.5));

	if (rtn != 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool Motion_Controller::Jog_Axis(int axis, double vel, double acc, double dec, double smooth)
{
	short rtn = GT_PrfJog(axis+1);

	if (rtn != 0)
	{
		return false;
	}

	TJogPrm jp;

	jp.acc = acc;
	jp.dec = dec;
	jp.smooth = smooth;

	rtn = GT_SetJogPrm(axis+1, &jp);

	if (rtn != 0)
	{
		return false;
	}

	rtn = GT_SetVel(axis+1, DPS_2_CPMS(axis, vel));

	if (rtn != 0)
	{
		return false;
	}

	return true;

}

bool Motion_Controller::P2P_Axis(int axis, double position, double vel, double vel_start, double acc, double dec, double smooth)
{
	short rtn = GT_PrfTrap(axis+1);

	if (rtn != 0)
	{
		return false;
	}

	TTrapPrm tp;
	tp.acc = acc;
	tp.dec = dec;
	tp.smoothTime = smooth;
	tp.velStart = DPS_2_CPMS(axis, vel_start);

	rtn = GT_SetTrapPrm(axis+1, &tp);

	if (rtn != 0)
	{
		return false;
	}



	rtn = GT_SetPos(axis+1, (long) floor(Physical_2_Count(axis, position)+0.5));
	
	if (rtn != 0)
	{
		
		return false;
	}



	rtn = GT_SetVel(axis+1, DPS_2_CPMS(axis, vel));

	if (rtn != 0)
	{
		return false;
	}

	return true;
}

bool Motion_Controller::Set_GPO_State(int pin, short state)
{
	short rtn = GT_SetDoBit(MC_GPO, pin+1, state);

	if (rtn != 0)
	{
		return false;
	}

	return true;
}

bool Motion_Controller::Get_GPI_State(int pin, int* state)
{
	long value;

	short rtn = GT_GetDi(MC_GPI, &value);

	if (rtn != 0)
	{
		return false;
	}

	if ((value & (1 << pin)) == (1 << pin))
	{
		*state = 1;
	}
	else
	{
		*state = 0;
	}

    return true;
}

bool Motion_Controller::Setup_Coordinate_System(int axis_count, double synvel_max, double synacc_max)
{
    short rtn;

    memset(&crd_prm, 0, sizeof(crd_prm));

    crd_prm.dimension = axis_count;
    crd_prm.synVelMax = DPS_2_CPMS(0, synvel_max); // arbitary use axis 0 to translate to count as all axes are expected to be the same
    crd_prm.synAccMax = synacc_max;
    crd_prm.evenTime = 50;
    crd_prm.setOriginFlag = 0;

    for (int i=0; i<axis_count; i++)
    {
        crd_prm.profile[i] = i+1;
        crd_prm.originPos[i] = 0;
    }

    rtn = GT_SetCrdPrm(1, &crd_prm);

    if (rtn != 0)
    {
       return false;
    }
    else
    {
        return true;
    }
}

bool Motion_Controller::Setup_Look_Ahead_Buffer(short crd, short fifo, double T, double synAccMax)
{
    short rtn;

    rtn = GT_InitLookAhead(crd, fifo, T, synAccMax, 200, this->crd_data );


    if (rtn != 0)
    {
        return false;
    }
    else
    {
        return true;
    }

}

bool Motion_Controller::Push_Circular_Motion_to_Buffer(short crd, short fifo, double xc, double yc, double x_end, double y_end, double synVel, double vel_end, double synAcc)
{
    short rtn;



    rtn = GT_ArcXYC(crd,
                    Physical_2_Count(0, x_end),
                    Physical_2_Count(1, y_end),
                    Physical_2_Count(0, xc),
                    Physical_2_Count(1, yc),
                    0, // always clockwise motion
                    DPS_2_CPMS(0, synVel),
                    synAcc,
                    vel_end,
                    fifo);

    if (rtn != 0)
    {
        return false;
    }
    else
    {
        return true;
    }

}

bool Motion_Controller::Push_XY_To_Buffer(short crd, short fifo, double x, double y, double synVel, double vel_end, double synAcc)
{
    short rtn;

    rtn = GT_LnXY(crd, Physical_2_Count(0, x),
                  Physical_2_Count(1, y),
                  DPS_2_CPMS(0, synVel),
                  synAcc,
                  vel_end,
                  fifo);

    if (rtn != 0)
    {
        return false;
    }
    else
    {
        return true;
    }

}

bool Motion_Controller::Transfer_Look_Ahead_To_Buffer(short crd, short fifo)
{
    short rtn;

    rtn = GT_CrdData(crd, NULL, fifo);

    if (rtn != 0)
    {
        return false;
    }
    else
    {
        return true;
    }


}

bool Motion_Controller::Clear_Buffer(short crd, short fifo)
{
    short rtn;

    rtn = GT_CrdClear(crd, fifo);

    if (rtn != 0)
    {
        return false;
    }
    else
    {
        return true;
    }

}

bool Motion_Controller::Get_Crd_Space(short crd, short fifo, long *space)
{
    short rtn;


    rtn = GT_CrdSpace(crd, space, fifo);

    if (rtn != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool Motion_Controller::Get_Remaining_Motion(short crd, short fifo, long *remaining)
{
    short rtn;

    rtn = GT_GetRemainderSegNum(crd, remaining, fifo);

    if (rtn != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool Motion_Controller::Start_Crd(short crd, short fifo)
{
    short rtn;

    rtn = GT_CrdStart(1, fifo);

    if (rtn != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool Motion_Controller::Get_Crd_Status(short crd, short fifo, short *is_motioning)
{
    short rtn;

    long segment;

    rtn = GT_CrdStatus(crd, is_motioning, &segment, fifo );

    if (rtn != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool Motion_Controller::Go_Axes(long mask)
{
	short rtn = GT_Update(mask);

	if (rtn != 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool Motion_Controller::Pt_Axis(int axis, int mode, int memory_mode)
{
	short rtn = GT_PrfPt(axis+1, mode);

	if (rtn != 0)
	{
		return false;
	}

	rtn = GT_SetPtMemory(axis+1, memory_mode);

	if (rtn != 0)
	{
		return false;
	}

	return true;
}

bool Motion_Controller::Clear_Pt(int axis, short fifo)
{
	short rtn = GT_PtClear(axis+1, fifo);

	if (rtn != 0)
	{
		return false;
	}
	else
	{
		return true;
	}

}

bool Motion_Controller::Get_Pt_Buffer_Space(int axis, short fifo, short* buffer_space)
{
	short space;

	short rtn = GT_PtSpace(axis+1, &space, fifo);

	if (rtn != 0)
	{
		return false;
	}
	else
	{
		*buffer_space = space;
		return true;
	}
}

bool Motion_Controller::Push_Physical_to_Pt_Buffer(int axis, double position, double time_ms, short fifo, short type, double reference)
{
	double ref_count = Physical_2_Count(axis,reference);
	double count = Physical_2_Count(axis, position);

	short rtn = GT_PtData(axis+1, count-ref_count, (long)  time_ms, type, fifo);

	//printf("%d %d %f\n\r,", axis, (long) time_ms, count);

	if (rtn != 0)
	{
		return false;
	}
	else
	{
		return true;
	}

}

bool Motion_Controller::Start_Pt(long mask, long option)
{
	short rtn = GT_PtStart(mask, option);

	if (rtn != 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool Motion_Controller::Sync_Profile_Encoder_Position(int axis, double set_position)
{
	Zero_Axis(axis);
	
	bool rtn = Set_Profile_Position(axis, set_position);

	if (rtn != true)
	{
		return false;
	}
	
	rtn = Set_Encoder_Position(axis, set_position);

	if (rtn != true)
	{
		return false;
	}

	short r = GT_SynchAxisPos(1 << axis);

	if (r != 0)
	{
		return false;
	}
	else
	{
		return true;
	}


}

bool Motion_Controller::Set_Soft_Limit(int axis, double positive_limit, double negative_limit)
{
	short rtn = GT_SetSoftLimit(axis+1, floor(Physical_2_Count(axis, positive_limit)+0.5), floor(Physical_2_Count(axis, negative_limit)+0.5)); 

	if (rtn != 0)
	{
		return false;
	}
	else
	{
		return true;
	}

	return true;
}

bool Motion_Controller::Abrupt_Stop_All_Axes()
{
	int temp = 0;

	for (int i=0; i<axis_count; i++)
	{
		temp = temp | (1 << i);
	}

	short rtn = GT_Stop(temp, temp);

	if (rtn != 0)
	{
		return false;
	}

	return true;
}

bool Motion_Controller::Smooth_Stop_All_Axes()
{
	int temp = 0;

	for (int i=0; i<axis_count; i++)
	{
		temp = temp | (1 << i);
	}

	short rtn = GT_Stop(temp, 0);

	if (rtn != 0)
	{
		return false;
	}

	return true;

}

bool Motion_Controller::Disable_Axis_Limit_Switches(int axis)
{
	short rtn = GT_LmtsOff(axis+1);

	printf("Limit switches of axis %d is disabled!\n\r", axis);

	if (rtn != 0)
	{
		return false;
	}
	else
	{
		return true;
	}

}

bool Motion_Controller::Enable_Axis_Limit_Switches(int axis)
{
	short rtn = GT_LmtsOn(axis+1);

	printf("Limit switches of axis %d is enabled!\n\r", axis);

	if (rtn != 0)
	{
		return false;
	}
	else
	{
		return true;
	}

}

bool Motion_Controller::Get_Axis_Status(int axis, long *status)
{
    short rtn = GT_GetSts(axis+1, status, 1);

    if (rtn != 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}


	


