#include "timer.h"

#include <GL/glfw.h>

Timer::Timer(bool start) :
    _running(start ? 1 : 0),
    _elapsed(0.0),
    _resumed(0.0)
{
    reset(start);
}

void Timer::pause()
{
    _running--;
    if (_running != 0)
    {
        return;
    }

    _elapsed += glfwGetTime() - _resumed;
}

void Timer::resume()
{
    _running++;
    if (_running != 1)
    {
        return;
    }

    _resumed = glfwGetTime();
}

void Timer::reset(bool start)
{
    _running = (start ? 1 : 0);
    if (start)
    {
        _resumed = glfwGetTime();
    }
    _elapsed = 0;
}

double Timer::read() const
{
    if (_running <= 0)
    {
        return _elapsed;
    }
    return glfwGetTime() - _resumed + _elapsed;
}
