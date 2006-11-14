#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include "common.h"
#include "video.h"

class FrameBuffer : public NoCopy
{
public:
    FrameBuffer();
    ~FrameBuffer();

    void create(unsigned int sizeX, unsigned int sizeY);
    void destroy();

    bool isValid() const;

    void bind() const;
    void unbind() const;

    unsigned int attachColorTex(bool hdr = false);
    unsigned int attachColorTex1(bool hdr = false);
    unsigned int attachDepthTex();
    void attachRBufDepth();

private:
    unsigned int m_sizeX;
    unsigned int m_sizeY;
    unsigned int m_colorTex;
    unsigned int m_colorTex1;
    unsigned int m_shadowTex;
    unsigned int m_shadowRBuf;
    unsigned int m_frameBuffer;
};

#endif
