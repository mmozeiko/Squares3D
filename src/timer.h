#ifndef __TIMER_H__
#define __TIMER_H__

#include "common.h"

class Timer : NoCopy
{
public:
    Timer(bool start = true);

    void pause();
    void resume();
    void reset(bool start = true);

    double read() const;

private:
    int     m_running;
    double  m_elapsed;
    double  m_resumed;
};

#endif
