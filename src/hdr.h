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
    int m_width;
    int m_height;
    
    Shader* m_final;

    GLuint m_sourceTex;
    GLuint m_downsampledTex;

    FrameBuffer* m_fboSource;
    FrameBuffer* m_fboDownsampled;
};

#endif
