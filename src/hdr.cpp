#include "hdr.h"
#include "shader.h"
#include "framebuffer.h"
#include "video.h"
#include "video_ext.h"

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
    try
    {
        m_downsample = new Shader("hdr_downsample");
        m_downsample->setInt1("tex_source", 0);
    }
    catch (const string& exception)
    {
        clog << "HDR not supported: " << exception << endl;
        return;
    }
    try
    {
        m_final = new Shader("hdr");
        m_final->setInt1("tex_source", 0);
        m_final->setInt1("tex_small", 1);
    }
    catch (const string& exception)
    {
        clog << "HDR not supported: " << exception << endl;
        return;
    }

    m_fboSource = new FrameBuffer();
    m_fboDownsampled = new FrameBuffer();

    int m = std::max(Video::instance->getResolution().first, Video::instance->getResolution().second);
    int pow2 = 1;
    while (pow2 < m)
    {
        pow2 <<= 1;
    }
    
    m_w = Video::instance->getResolution().first / static_cast<float>(pow2);
    m_h = Video::instance->getResolution().second / static_cast<float>(pow2);

    m_fboSource->create(pow2);
    m_sourceTex = m_fboSource->attachColorTex();
    m_fboSource->attachDepthTex(true);
    if (!m_fboSource->isValid())
    {
        return;
    }

    m_fboDownsampled->create(256);
    m_downsampledTex = m_fboDownsampled->attachColorTex(true);
    if (!m_fboDownsampled->isValid())
    {
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
        glTexCoord2f(0.0f, 0.0f); glVertex2i(0, 0);
        glTexCoord2f(1.0f, 0.0f); glVertex2i(1, 0);
        glTexCoord2f(1.0f, 1.0f); glVertex2i(1, 1);
        glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 1);
    glEnd();
    m_downsample->end();
    m_fboDownsampled->unbind();

    glViewport(m_view[0], m_view[1], m_view[2], m_view[3]);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, m_sourceTex);
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, m_downsampledTex);
    glEnable(GL_TEXTURE_2D);

    /*vector<char> data(512*512*3);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);
    FILE* f = fopen("xxx.raw", "wb");
    fwrite(&data[0], data.size(), 1, f);
    fclose(f);*/

    m_final->begin();
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2i(0, 0);  
        glTexCoord2f(m_w, 0.0f);  glVertex2i(1, 0); 
        glTexCoord2f(m_w, m_h);   glVertex2i(1, 1);  
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
