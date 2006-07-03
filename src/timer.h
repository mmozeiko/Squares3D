#ifndef __TIMER_H__
#define __TIMER_H__

class Timer
{
public:
    Timer(bool start = true);

    void pause();
    void resume();
    void reset(bool start = true);

    double read() const;

private:
    int _running;

    double _elapsed;
    double _resumed;
};

#endif
