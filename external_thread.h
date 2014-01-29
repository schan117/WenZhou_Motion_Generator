#ifndef EXTERNAL_THREAD_H
#define EXTERNAL_THREAD_H

#include "Motion_Controller.h"
#include "Script_Reader.h"
#include <QThread>


class External_Thread : public QThread
{

Q_OBJECT

public:
    External_Thread();
    \
    void Initialize(Motion_Controller* mmc, Script_Reader* sr, double syn_Acc);

    void run();

    Motion_Controller* mc;

    bool keep_motioning;

    double synAcc;

    bool pause;

    Script_Reader* ssr;

     int current_line;

private:

    void Next_Line();

};

#endif // EXTERNAL_THREAD_H
