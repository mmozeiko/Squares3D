#ifndef __TIMER_H__
#define __TIMER_H__

#include "common.h"

class Timer : public NoCopy
{
public:
    Timer(bool start = true);

    void pause();
    void resume();
    void reset(bool start = true);

    float read() const;

private:
    int    m_running;
    float  m_elapsed;
    float  m_resumed;
};

#endif
