#ifndef __HDR_H__
#define __HDR_H__

#include "common.h"
#include "vmath.h"

#include "video_ext.h"

class Shader;
class FrameBuffer;

class HDR : NoCopy
{
public:
    HDR();
    ~HDR();

    void init();

    void begin();
    void end();
    void render();

private:
    bool m_valid;

    float m_h;
    float m_w;
    
    Shader* m_downsample;
    Shader* m_final;

    GLuint m_sourceTex;
    GLuint m_downsampledTex256;
    GLuint m_downsampledTex128;
    GLuint m_downsampledTex64;

    FrameBuffer* m_fboSource;
    FrameBuffer* m_fboDownsampled256;
    FrameBuffer* m_fboDownsampled128;
    FrameBuffer* m_fboDownsampled64;
};

#endif
