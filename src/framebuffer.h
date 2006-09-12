#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include "common.h"

class FrameBuffer : NoCopy
{
public:
    FrameBuffer();
    ~FrameBuffer();

    void create(unsigned int size);
    void destroy();

    bool isValid() const;

    void bind() const;
    void unbind() const;

    unsigned int attachColorTex();
    unsigned int attachDepthTex();

private:
    unsigned int m_size;
    unsigned int m_colorTex;
    unsigned int m_shadowTex;
    unsigned int m_frameBuffer;
    unsigned int m_renderBuffer;
};

#endif