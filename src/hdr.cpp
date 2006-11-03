#include "hdr.h"
#include "shader.h"
#include "framebuffer.h"
#include "video.h"
#include "video_ext.h"
#include "config.h"

HDR::HDR() :
    m_downsample(NULL),
    m_blurH(NULL),
    m_blurV(NULL),
    m_final(NULL),
    m_sourceTex(0),
    m_downsampledTex256_0(0),
    m_downsampledTex128_0(0),
    m_downsampledTex64_0(0),
    m_downsampledTex256_1(0),
    m_downsampledTex128_1(0),
    m_downsampledTex64_1(0),
    m_fboSource(NULL),
    m_fboDownsampled256(NULL),
    m_fboDownsampled128(NULL),
    m_fboDownsampled64(NULL),
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
        m_blurH = new Shader("hdr_blurH");
        m_blurH->setInt1("tex_source", 0);
        m_blurH->end();
    }
    catch (const string& exception)
    {
        clog << "HDR not supported: " << exception << endl;
        Config::instance->m_video.use_hdr = false;
        return;
    }

    try
    {
        m_blurV = new Shader("hdr_blurV");
        m_blurV->setInt1("tex_source", 0);
        m_blurV->end();
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
        m_final->setInt1("tex_small_256", 1);
        m_final->setInt1("tex_small_128", 2);
        m_final->setInt1("tex_small_64", 3);
        m_final->end();
    }
    catch (const string& exception)
    {
        clog << "HDR not supported: " << exception << endl;
        Config::instance->m_video.use_hdr = false;
        return;
    }

    m_fboSource = new FrameBuffer();
    m_fboDownsampled256 = new FrameBuffer();
    m_fboDownsampled128 = new FrameBuffer();
    m_fboDownsampled64 = new FrameBuffer();

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

    m_fboDownsampled256->create(256, 256);
    m_downsampledTex256_0 = m_fboDownsampled256->attachColorTex(true);
    m_downsampledTex256_1 = m_fboDownsampled256->attachColorTex1(true);
    
    m_fboDownsampled128->create(128, 128);
    m_downsampledTex128_0 = m_fboDownsampled128->attachColorTex(true);
    m_downsampledTex128_1 = m_fboDownsampled128->attachColorTex1(true);
    
    m_fboDownsampled64->create(64, 64);   
    m_downsampledTex64_0 = m_fboDownsampled64->attachColorTex(true);
    m_downsampledTex64_1 = m_fboDownsampled64->attachColorTex1(true);
    
    if (!m_fboDownsampled256->isValid() || !m_fboDownsampled128->isValid() || !m_fboDownsampled64->isValid())
    {
        Config::instance->m_video.use_hdr = false;
        return;
    }

    m_valid = true;
}

HDR::~HDR()
{
    if (m_downsample != NULL) delete m_downsample;
    if (m_blurH != NULL) delete m_blurH;
    if (m_blurV != NULL) delete m_blurV;
    if (m_final != NULL) delete m_final;
    if (m_fboSource != NULL) delete m_fboSource;
    if (m_fboDownsampled256 != NULL) delete m_fboDownsampled256;
    if (m_fboDownsampled128 != NULL) delete m_fboDownsampled128;
    if (m_fboDownsampled64 != NULL) delete m_fboDownsampled64;
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

    GLint draw_buffer;
    glGetIntegerv(GL_DRAW_BUFFER, &draw_buffer);
    
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    int m_view[4];
    glGetIntegerv(GL_VIEWPORT, m_view);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, m_sourceTex);



    
    m_downsample->begin();

    glViewport(0, 0, 256, 256);
    m_fboDownsampled256->bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(m_w, 0.0f);  glVertex2f(1.0f, 0.0f);
        glTexCoord2f(m_w, m_h);   glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, m_h);  glVertex2f(0.0f, 1.0f);
    glEnd();
    m_fboDownsampled256->unbind();

    glViewport(0, 0, 128, 128);
    m_fboDownsampled128->bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(m_w, 0.0f);  glVertex2f(1.0f, 0.0f);
        glTexCoord2f(m_w, m_h);   glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, m_h);  glVertex2f(0.0f, 1.0f);
    glEnd();
    m_fboDownsampled128->unbind();

    glViewport(0, 0, 64, 64);
    m_fboDownsampled64->bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(m_w, 0.0f);  glVertex2f(1.0f, 0.0f);
        glTexCoord2f(m_w, m_h);   glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, m_h);  glVertex2f(0.0f, 1.0f);
    glEnd();
    m_fboDownsampled64->unbind();

    m_downsample->end();



    m_blurH->begin();

    glBindTexture(GL_TEXTURE_2D, m_downsampledTex256_0);

    m_blurH->setFloat1("tex_small_size", 256.0f);
    m_blurH->setFloat1("divider", 115.0f * 2.0f);
    glViewport(0, 0, 256, 256);
    m_fboDownsampled256->bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
    glEnd();
    m_fboDownsampled256->unbind();

    glBindTexture(GL_TEXTURE_2D, m_downsampledTex128_0);

    m_blurH->setFloat1("tex_small_size", 128.0f);
    m_blurH->setFloat1("divider", 115.0f * 1.5f);
    glViewport(0, 0, 128, 128);
    m_fboDownsampled128->bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
    glEnd();
    m_fboDownsampled128->unbind();

    glBindTexture(GL_TEXTURE_2D, m_downsampledTex64_0);

    m_blurH->setFloat1("tex_small_size", 64.0f);
    m_blurH->setFloat1("divider", 115.0f);
    glViewport(0, 0, 64, 64);
    m_fboDownsampled64->bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
    glEnd();
    m_fboDownsampled64->unbind();

    m_blurH->end();



    m_blurV->begin();

    glBindTexture(GL_TEXTURE_2D, m_downsampledTex256_1);

    m_blurV->setFloat1("tex_small_size", 256.0f);
    m_blurV->setFloat1("divider", 115.0f * 2.0f);
    glViewport(0, 0, 256, 256);
    m_fboDownsampled256->bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
    glEnd();
    m_fboDownsampled256->unbind();

    glBindTexture(GL_TEXTURE_2D, m_downsampledTex128_1);

    m_blurV->setFloat1("tex_small_size", 128.0f);
    m_blurV->setFloat1("divider", 115.0f * 1.5f);
    glViewport(0, 0, 128, 128);
    m_fboDownsampled128->bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
    glEnd();
    m_fboDownsampled128->unbind();

    glBindTexture(GL_TEXTURE_2D, m_downsampledTex64_1);

    m_blurV->setFloat1("tex_small_size", 64.0f);
    m_blurV->setFloat1("divider", 115.0f);
    glViewport(0, 0, 64, 64);
    m_fboDownsampled64->bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
    glEnd();
    m_fboDownsampled64->unbind();

    m_blurV->end();



    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, m_sourceTex);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, m_downsampledTex256_0);
    glEnable(GL_TEXTURE_2D);
    
    glActiveTextureARB(GL_TEXTURE2_ARB);
    glBindTexture(GL_TEXTURE_2D, m_downsampledTex128_0);
    glEnable(GL_TEXTURE_2D);
    
    glActiveTextureARB(GL_TEXTURE3_ARB);
    glBindTexture(GL_TEXTURE_2D, m_downsampledTex64_0);
    glEnable(GL_TEXTURE_2D);

    glViewport(m_view[0], m_view[1], m_view[2], m_view[3]);
    glDrawBuffer(draw_buffer);

    m_final->begin();
    glBegin(GL_QUADS);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 0.0f);
        glVertex2f(0.0f, 0.0f);  

        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, m_w,  0.0f);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 0.0f);
        glVertex2f(1.0f, 0.0f); 

        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, m_w,  m_h);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 1.0f);
        glVertex2f(1.0f, 1.0f);  
        
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, m_h);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 1.0f);
        glVertex2f(0.0f, 1.0f);   
    glEnd();
    m_final->end();

    glActiveTextureARB(GL_TEXTURE3_ARB);
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE2_ARB);
    glDisable(GL_TEXTURE_2D);
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
