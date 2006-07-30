#include "fps.h"
#include "font.h"
#include "timer.h"
#include <cmath>
#include <GL/glfw.h>

FPS::FPS(const Timer& timer, const Font& font)
    : m_frames(0), m_timer(timer), m_font(font), m_fps()
{
}

void FPS::update()
{
    if ((m_frames<64 && m_frames%16==0) || m_frames%128==0)
    {
        m_fps = "FPS: " + cast<string>(fps());
    }
    m_frames++;
}


void FPS::render() const
{
    m_font.begin();
    glTranslatef(0.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    m_font.render(m_fps);
    m_font.end();
}

unsigned int FPS::frames() const
{
    return m_frames;
}

double FPS::time() const
{
    return m_timer.read();
}

double FPS::fps() const
{
    double fps = m_frames/m_timer.read();
    return std::floor(fps*10.0)/10.0;
}
