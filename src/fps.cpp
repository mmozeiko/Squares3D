#include <cmath>

#include "fps.h"
#include "font.h"
#include "timer.h"
#include "config.h"
#include "video.h"

FPS::FPS(const Timer& timer, const Font* font, const Vector& color)
    : m_time(timer.read()), m_frames(0), m_totalFrames(0), m_timer(timer), m_font(font), m_fps(),
    color(color)
{
}

void FPS::update()
{
    m_frames++;
    if (m_frames%128==0 || m_totalFrames<2)
    {
        float curTime = m_timer.read();
        float fps = m_frames/(curTime-m_time);
        fps = std::floor(fps*10.0f)/10.0f;
        m_fps = "FPS: " + cast<string>(fps) + (fps-std::floor(fps)==0.0 ? ".0" : "");
        m_size = m_font->getSize(m_fps);
        m_time = curTime;
        m_frames = 0;
    }
    m_totalFrames++;
}


void FPS::render() const
{
    m_font->begin();

    IntPair res = Video::instance->getResolution();

    glTranslatef(
        static_cast<float>(res.first - m_size.first)/2, 
        static_cast<float>(res.second - m_size.second),
        0.0f);
    glColor3fv(color.v);
    m_font->render(m_fps);
    m_font->end();
}

unsigned int FPS::frames() const
{
    return m_totalFrames;
}

float FPS::time() const
{
    return m_timer.read();
}

float FPS::fps() const
{
    float fps = m_totalFrames/m_timer.read();
    return std::floor(fps*10.0f)/10.0f;
}
