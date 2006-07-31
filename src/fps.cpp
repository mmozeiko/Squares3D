#include <cmath>
#include <GL/glfw.h>

#include "fps.h"
#include "font.h"
#include "timer.h"

FPS::FPS(const Timer& timer, const Font& font, const Vector& color)
    : m_time(timer.read()), m_frames(0), m_totalFrames(0), m_timer(timer), m_font(font), m_fps(),
    color(color)
{
}

void FPS::update()
{
    m_frames++;
    if (m_frames%128==0)
    {
        double curTime = m_timer.read();
        double fps = m_frames/(curTime-m_time);
        fps = std::floor(fps*10.0)/10.0;
        m_fps = "FPS: " + cast<string>(fps) + (fps-std::floor(fps)==0.0 ? ".0" : "");
        m_time = curTime;
        m_frames = 0;
    }
    m_totalFrames++;
}


void FPS::render() const
{
    m_font.begin();
    glTranslatef(0.0f, 0.0f, 0.0f);
    glColor3fv(color.v);
    m_font.render(m_fps);
    m_font.end();
}

unsigned int FPS::frames() const
{
    return m_totalFrames;
}

double FPS::time() const
{
    return m_timer.read();
}

double FPS::fps() const
{
    double fps = m_totalFrames/m_timer.read();
    return std::floor(fps*10.0)/10.0;
}
