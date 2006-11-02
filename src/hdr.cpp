#include "hdr.h"
#include "shader.h"
#include "framebuffer.h"
#include "video.h"
#include "video_ext.h"
#include "config.h"

HDR::HDR() :
    m_downsample(NULL),
    m_final(NULL),
    m_sourceTex(0),
    m_downsampledTex(0),
    m_fboSource(NULL),
    m_fboDownsampled(NULL),
    m_w(0.0f),
    m_h(0.0f),
    m_valid(false)
{
}

void HDR::init()
{
    if (Config::instance->m_video.use_hdr == false)
    {
        return;
    }

    try
    {
        m_downsample = new Shader("hdr_downsample");
        m_downsample->setInt1("tex_source", 0);
        m_downsample->end();
    }
    catch (const string& exception)
    {
        clog << "HDR not supported: " << exception << endl;
        Config::instance->m_video.use_hdr = false;
        return;
    }
    try
    {
        m_final = new Shader("hdr_final");
        m_final->setInt1("tex_source", 0);
        m_final->setInt1("tex_small", 1);
        m_final->setFloat1("tex_small_size", 256.0f);
        m_final->end();
    }
    catch (const string& exception)
    {
        clog << "HDR not supported: " << exception << endl;
        Config::instance->m_video.use_hdr = false;
        return;
    }

    m_fboSource = new FrameBuffer();
    m_fboDownsampled = new FrameBuffer();

    const int x = Video::instance->getResolution().first;
    int pow2x = 1;
    while (pow2x < x)
    {
        pow2x <<= 1;
    }
    
    const int y = Video::instance->getResolution().second;
    int pow2y = 1;
    while (pow2y < y)
    {
        pow2y <<= 1;
    }
    
    m_w = Video::instance->getResolution().first / static_cast<float>(pow2x);
    m_h = Video::instance->getResolution().second / static_cast<float>(pow2y);

    m_fboSource->create(pow2x, pow2y);
    m_sourceTex = m_fboSource->attachColorTex();
    m_fboSource->attachDepthTex(true);
    if (!m_fboSource->isValid())
    {
        Config::instance->m_video.use_hdr = false;
        return;
    }

    m_fboDownsampled->create(256, 256);
    m_downsampledTex = m_fboDownsampled->attachColorTex(true);
    if (!m_fboDownsampled->isValid())
    {
        Config::instance->m_video.use_hdr = false;
        return;
    }

    m_valid = true;
}

HDR::~HDR()
{
    if (m_downsample != NULL) delete m_downsample;
    if (m_final != NULL) delete m_final;
    if (m_fboSource != NULL) delete m_fboSource;
    if (m_fboDownsampled != NULL) delete m_fboDownsampled;
}

void HDR::begin()
{
    if (!m_valid)
    {
        return;
    }
    
    m_fboSource->bind();
}

void HDR::end()
{
    if (!m_valid)
    {
        return;
    }
    
    m_fboSource->unbind();
}

void HDR::render()
{
    if (!m_valid)
    {
        return;
    }
    
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    int m_view[4];
    glGetIntegerv(GL_VIEWPORT, m_view);

    glViewport(0, 0, 256, 256);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, m_sourceTex);
    glEnable(GL_TEXTURE_2D);

    m_fboDownsampled->bind();
    m_downsample->begin();
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(m_w, 0.0f);  glVertex2f(m_w,  0.0f);
        glTexCoord2f(m_w, m_h);   glVertex2f(m_w,  m_h);
        glTexCoord2f(0.0f, m_h);  glVertex2f(0.0f, m_h);
    glEnd();
    m_downsample->end();
    m_fboDownsampled->unbind();

    glViewport(m_view[0], m_view[1], m_view[2], m_view[3]);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, m_sourceTex);
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, m_downsampledTex);
    glEnable(GL_TEXTURE_2D);

    m_final->begin();
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2i(0, 0);  
        glTexCoord2f(m_w,  0.0f); glVertex2i(1, 0); 
        glTexCoord2f(m_w,  m_h);  glVertex2i(1, 1);  
        glTexCoord2f(0.0f, m_h);  glVertex2i(0, 1);   
    glEnd();
    m_final->end();

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);

    glActiveTextureARB(GL_TEXTURE0_ARB);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
}
