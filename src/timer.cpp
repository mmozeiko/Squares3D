#include "timer.h"

#include <GL/glfw.h>

Timer::Timer(bool start) :
    m_running(start ? 1 : 0),
    m_elapsed(0.0),
    m_resumed(0.0)
{
    reset(start);
}

void Timer::pause()
{
    m_running--;
    if (m_running != 0)
    {
        return;
    }

    m_elapsed += glfwGetTime() - m_resumed;
}

void Timer::resume()
{
    m_running++;
    if (m_running != 1)
    {
        return;
    }

    m_resumed = glfwGetTime();
}

void Timer::reset(bool start)
{
    m_running = (start ? 1 : 0);
    if (start)
    {
        m_resumed = glfwGetTime();
    }
    m_elapsed = 0;
}

double Timer::read() const
{
    if (m_running <= 0)
    {
        return m_elapsed;
    }
    return glfwGetTime() - m_resumed + m_elapsed;
}
