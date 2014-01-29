#ifndef CIRCULAR_THREAD_H
#define CIRCULAR_THREAD_H

#include <QThread>
#include "Motion_Controller.h"
#include <QDoubleSpinBox>

class Circular_Thread : public QThread
{

Q_OBJECT

public:
    Circular_Thread();

    void Initialize(Motion_Controller* mmc, QDoubleSpinBox* diameter, QDoubleSpinBox* frequency, double syn_Acc);

    void run();

    Motion_Controller* mc;
    QDoubleSpinBox* dia;
    QDoubleSpinBox* freq;

    bool keep_circling;

    double synAcc;

    bool pause;

};

#endif // CIRCULAR_THREAD_H
