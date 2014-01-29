#pragma once

#include <QtCore>


#define	MOVEABS			0
#define	MOVEVAL			1
#define	INDEX			2
#define	GPO				3
#define	GPI				4
#define	DELAY			5
#define	INDEXOFFSET		6
#define	TCPO			7
#define	TCPI			8
#define	MOVEZ			9
#define MOVEXYOFFSET	10
#define IF				11
#define	END				12
#define	INDEXNM			13
#define SETVAL			14
#define MOVEABSS		15
#define	INSPECT			16
#define READGPI			17
#define	PAUSEGPI		18
#define MOVETXY         19

#define ERR_OK                      0
#define ERR_LINE_COUNT              1
#define ERR_FIRST_LAST_NOT_EQUAL    2
#define ERR_OUT_OF_RANGE            3
#define ERR_SYNTAX                  4



struct Script_Step
{
	int type;
	QVariant params[20];
};


class Script_Reader
{



public:
	Script_Reader(void);
	~Script_Reader(void);

	bool Read_Script_File(QString file);
	void Clear_Internal_Script();

    int Check_Internal_Script_XY_Table(int min_line_count, double min_t, double min_x, double max_x, double min_y, double max_y, double max_vel_allowed, int* first_error_line);


	QList<Script_Step> script;
	
	bool previous_read_ok;
};

