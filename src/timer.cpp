#include <GL/glfw.h>

#include "timer.h"

Timer::Timer(bool start) :
    m_running(start ? 1 : 0),
    m_elapsed(0.0f),
    m_resumed(0.0f)
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

    m_elapsed += static_cast<float>(glfwGetTime()) - m_resumed;
}

void Timer::resume()
{
    m_running++;
    if (m_running != 1)
    {
        return;
    }

    m_resumed = static_cast<float>(glfwGetTime());
}

void Timer::reset(bool start)
{
    m_running = (start ? 1 : 0);
    if (start)
    {
        m_resumed = static_cast<float>(glfwGetTime());
    }
    m_elapsed = 0;
}

float Timer::read() const
{
    if (m_running <= 0)
    {
        return m_elapsed;
    }
    return static_cast<float>(glfwGetTime()) - m_resumed + m_elapsed;
}
