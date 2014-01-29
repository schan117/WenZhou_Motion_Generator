#include "Script_Reader.h"


Script_Reader::Script_Reader(void)
{
	previous_read_ok = false;
}


Script_Reader::~Script_Reader(void)
{
}

bool Script_Reader::Read_Script_File(QString file)
{
	previous_read_ok = false;

    //bool keep_reading = true;

	QFile f(file);
	if  (!f.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		return false;
	}

	QTextStream s(&f);

	Script_Step step;
	QString line;
	QStringList list;

	int current_line = 0;
	bool line_ok = false;
	bool comment_line = false;

	script.clear();

	while (!s.atEnd())
	{
		line = s.readLine();

		list = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);

    	if (list.count() != 0) // for non-empty list
		{
			if (list[0].compare("MoveAbs", Qt::CaseInsensitive) == 0)
			{
				if (list.count() != 4)
				{
                    qDebug("Line %d: MoveAbs, Parameters error!!", current_line);
					Clear_Internal_Script();
					return false;
				}

                qDebug("Line %d: MoveAbs, params: ", current_line);
				step.type = MOVEABS;
                QString temp;
				for (int j=1; j<list.count(); j++)
				{
                    temp = temp + list[j] + " ";
				}
                qDebug() << temp;

				line_ok = true;
			} 


			else if (list[0].compare("GPO", Qt::CaseInsensitive) == 0)
			{
				if (list.count() != 3)
				{
                    qDebug("Line %d: GPO, Parameters error!!\n\r", current_line);
					Clear_Internal_Script();
					return false;
				}

                qDebug("Line %d: GPO, params: ", current_line);
				step.type = GPO;
                QString temp;
                for (int j=1; j<list.count(); j++)
                {
                    temp = temp + list[j] + " ";
                }
                qDebug() << temp;
				line_ok = true;
			}
			else if (list[0].compare("GPI", Qt::CaseInsensitive) == 0)
			{
				// check parameters count
				if (list.count() != 4)
				{
                    qDebug("Line %d: GPI, Parameters error!!\n\r", current_line);
					Clear_Internal_Script();
					return false;
				}
				
                qDebug("Line %d: GPI, params: ", current_line);
				step.type = GPI;
                QString temp;
                for (int j=1; j<list.count(); j++)
                {
                    temp = temp + list[j] + " ";
                }
                qDebug() << temp;
				line_ok = true;
			}
			else if (list[0].compare("DELAY", Qt::CaseInsensitive) == 0)
			{
				// check parameters count
				if (list.count() != 2)
				{
                    qDebug("Line %d: Delay, Parameters error!!\n\r", current_line);
					Clear_Internal_Script();
					return false;
				}

                qDebug("Line %d: Delay, params: ", current_line);
				
				step.type = DELAY;
                QString temp;
                for (int j=1; j<list.count(); j++)
                {
                    temp = temp + list[j] + " ";
                }
                qDebug() << temp;
				line_ok = true;
			}
            else if (list[0].compare("MOVETXY", Qt::CaseInsensitive) == 0)
            {
                // check parameters count
                if (list.count() != 4)
                {
                    qDebug("Line %d: MoveTXY, Parameters error!!\n\r", current_line);
                    Clear_Internal_Script();
                    return false;
                }

                qDebug("Line %d: MoveTXY, params: ", current_line);

                step.type = MOVETXY;
                QString temp;
                for (int j=1; j<list.count(); j++)
                {
                    temp = temp + list[j] + " ";
                }
                qDebug() << temp;
                line_ok = true;
            }

			else if (list[0].compare("ENDSCRIPT", Qt::CaseInsensitive) == 0)
			{
				// break while loop if eof detected
                qDebug("End of Script");
				break;
			}
		}

		if (line_ok)
		{
	    	// fill in parameters
			if (!comment_line)
			{
				for (int i=1; i<list.count(); i++)
				{
					step.params[i-1] = list[i];
				}

				script.append(step);
				line_ok = false;
				comment_line = false;

				current_line++;
			}
			else
			{
				// reset for next line
				line_ok = false;
				comment_line = false;
			}
		}
		else
		{
            qDebug("Line %d: Invalid, not appending to script list", current_line);
			Clear_Internal_Script();
			return false;
		}
	}

    qDebug("Total %d lines in script", script.count());

	f.close();

	previous_read_ok = true;

	return true;
}

void Script_Reader::Clear_Internal_Script()
{
    qDebug("Clear internal script!!");
    script.clear();
}

int Script_Reader::Check_Internal_Script_XY_Table(int min_line_count, double min_t, double min_x, double max_x, double min_y, double max_y, double max_vel_allowed, int* first_error_line)
{
    // check for validity ofor internal script here

    // first check for empty script
    if (script.count() < min_line_count)
    {
        // it is expected that at least there will be 3 lines for XY Table motion

        return ERR_LINE_COUNT;
    }


    //  check whether coordinates are all valid

    for (int i=0; i<script.count(); i++)
    {
        bool ok;

        // check T

        double t = script[i].params[0].toDouble(&ok);
        if (!ok)
        {
            *first_error_line = i;
            return ERR_SYNTAX;
        }

        // check if T is positive

        if (t < min_t)
        {
            *first_error_line = i;
            return ERR_OUT_OF_RANGE;
        }

        // check X

        double x = script[i].params[1].toDouble(&ok);
        if (!ok)
        {
            *first_error_line = i;
            return ERR_SYNTAX;
        }

        // check x is within range

        if (    (x < min_x) || (x > max_x)  )
        {
            *first_error_line = i;
            return ERR_OUT_OF_RANGE;
        }

        // check Y

        double y = script[i].params[2].toDouble(&ok);
        if (!ok)
        {
            *first_error_line = i;
            return ERR_SYNTAX;
        }

        // check y is within range

        if (    (y < min_y) || (y > max_y)  )
        {
            *first_error_line = i;
            return ERR_OUT_OF_RANGE;
        }
    }

    // coordinates of first line must equal that of last line

    if (    (script[0].params[1].toDouble() != script[script.count()-1].params[1].toDouble()) ||   // x
            (script[0].params[2].toDouble() != script[script.count()-1].params[2].toDouble()) )    // y
    {
         return ERR_FIRST_LAST_NOT_EQUAL;
    }


    // check for time sequence, that is, are they all increasing, if so, calculate velocity needed

    for (int i=1; i<script.count(); i++)
    {
        double dt = script[i].params[0].toDouble() - script[i-1].params[0].toDouble();

        if (dt < 0)
        {
            *first_error_line = i-1;
            return ERR_OUT_OF_RANGE;
        }

        double dx, dy;

        dx = script[i].params[1].toDouble() - script[i-1].params[1].toDouble();
        dy = script[i].params[2].toDouble() - script[i-1].params[2].toDouble();

        double dist = sqrt(dx*dx + dy*dy);

        double v = dist / dt;

        if (v > max_vel_allowed) // in mm / s
        {
            // revert back to max_vel_allowed

            script[i].params[3] = max_vel_allowed;
        }
        else
        {
            script[i].params[3] = v;
        }
    }

    // All checking done here

    for (int i=1; i<script.count(); i++)
    {
        qDebug() << "Line:" << i << "X:" << script[i].params[1].toDouble() << "Y:" << script[i].params[2].toDouble() << "V:" << script[i].params[3].toDouble();
    }

    return ERR_OK;

}
