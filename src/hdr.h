#ifndef __HDR_H__
#define __HDR_H__

#include "common.h"
#include "vmath.h"
#include "video.h"

class Shader;
class FrameBuffer;

static const int hdr_levels = 3; // starting from 256

class HDR : NoCopy
{
public:
    HDR();
    ~HDR();

    void init();
    void updateFromLevel(float eps, const Vector& mul);

    void begin();
    void end();
    void render();

private:
    bool m_valid;

    float m_h;
    float m_w;
   
    Shader* m_downsample;
    Shader* m_blur;
    Shader* m_final;

    GLuint m_sourceTex;
    GLuint m_downsampledTex[hdr_levels][2];

    FrameBuffer* m_fboSource;
    FrameBuffer* m_fboDownsampled[hdr_levels];
};

#endif
