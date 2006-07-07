#include "timer.h"

#include <GL/glfw.h>

Timer::Timer(bool start) :
    _running(start ? 1 : 0),
    _elapsed(0.0),
    _resumed(0.0)
{
    Reset(start);
}

void Timer::Pause()
{
    _running--;
    if (_running != 0)
    {
        return;
    }

    _elapsed += glfwGetTime() - _resumed;
}

void Timer::Resume()
{
    _running++;
    if (_running != 1)
    {
        return;
    }

    _resumed = glfwGetTime();
}

void Timer::Reset(bool start)
{
    _running = (start ? 1 : 0);
    if (start)
    {
        _resumed = glfwGetTime();
    }
    _elapsed = 0;
}

double Timer::Read() const
{
    if (_running <= 0)
    {
        return _elapsed;
    }
    return glfwGetTime() - _resumed + _elapsed;
}
