#ifndef __FPS_H__
#define __FPS_H__

#include "common.h"

class Font;
class Timer;

class FPS
{
public:
    FPS(const Timer& timer, const Font& font);
    void update();
    void render() const;

    unsigned int frames() const;
    double time() const;
    double fps() const;

private:
    unsigned int m_frames;
    const Timer& m_timer;
    const Font& m_font;
    string m_fps;
};

#endif
