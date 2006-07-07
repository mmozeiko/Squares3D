#ifndef __TIMER_H__
#define __TIMER_H__

class Timer
{
public:
    Timer(bool start = true);

    void Pause();
    void Resume();
    void Reset(bool start = true);

    double Read() const;

private:
    int _running;

    double _elapsed;
    double _resumed;
};

#endif
