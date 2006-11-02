#include "hdr.h"
#include "shader.h"
#include "framebuffer.h"
#include "video.h"
#include "video_ext.h"

HDR::HDR() :
    m_width(0),
    m_height(0),
    m_final(NULL),
    m_sourceTex(0),
    m_downsampledTex(0),
    m_fboSource(NULL),
    m_fboDownsampled(NULL)
{
}

void HDR::init()
{
    //m_blur = new Shader("hdr_blur");
    //m_blur->setInt1("tex_hdr", 0);

    m_final = new Shader("hdr_final");
    m_final->setInt1("tex_source", 0);
    m_final->setInt1("tex_blur", 1);

    m_fboSource = new FrameBuffer();
    m_fboDownsampled = new FrameBuffer();

    m_fboSource->create(1024);
    m_sourceTex = m_fboSource->attachColorTex();
    m_fboSource->attachDepthTex();
    clog << "VALID = " << m_fboSource->isValid() << endl;

    m_fboDownsampled->create(256);
    m_downsampledTex = m_fboDownsampled->attachColorTex(true);
    clog << "VALID = " << m_fboDownsampled->isValid() << endl;
}

HDR::~HDR()
{
    if (m_final != NULL) delete m_final;
    if (m_fboSource != NULL) delete m_fboSource;
    if (m_fboDownsampled != NULL) delete m_fboDownsampled;
}

void HDR::begin()
{
    m_fboSource->bind();
}

void HDR::end()
{
    m_fboSource->unbind();
}

void HDR::render()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    int m_view[4];
    glGetIntegerv(GL_VIEWPORT, m_view);

    glViewport(0, 0, 256, 256);

    float w_aspect = Video::instance->getResolution().first / 1024.0f;
    float h_aspect = Video::instance->getResolution().second / 1024.0f;

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
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);        glVertex2i(0, 0);
        glTexCoord2f(1.0f, 0.0f);        glVertex2i(1, 0);
        glTexCoord2f(1.0f, 1.0f);        glVertex2i(1, 1);
        glTexCoord2f(0.0f, 1.0f);        glVertex2i(0, 1);
    glEnd();
    m_fboDownsampled->unbind();

    //int GL_TEXTURE_RECTANGLE_ARB = 0x84F5;
   /*
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, m_downsampledTex);

    m_fboH->bind();
    m_blur->setFloat4("blur_offset", Vector(4.0f/512.0f, 0, 0, 1));
    m_blur->begin();
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);        glVertex2i(0, 0);
        glTexCoord2f(1.0f, 0.0f);        glVertex2i(1, 0);
        glTexCoord2f(1.0f, 1.0f);        glVertex2i(1, 1);
        glTexCoord2f(0.0f, 1.0f);        glVertex2i(0, 1);
    glEnd();

    m_blur->end();
    m_fboH->unbind();

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, m_blurHtex);
    m_fboV->bind();
    m_blur->setFloat4("blur_offset", Vector(0, 4.0f/512.0f, 0, 1));
    m_blur->begin();
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2i(0, 0);
        glTexCoord2f(1.0f, 0.0f); glVertex2i(1, 0);
        glTexCoord2f(1.0f, 1.0f); glVertex2i(1, 1);
        glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 1);
    glEnd();

    m_blur->end();
    m_fboV->unbind();
*/
    glViewport(m_view[0], m_view[1], m_view[2], m_view[3]);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, m_sourceTex);
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, m_downsampledTex);
    glEnable(GL_TEXTURE_2D);

    m_final->begin();
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);         glVertex2i(-1, -1);  
        glTexCoord2f(w_aspect, 0.0f);     glVertex2i(1, -1); 
        glTexCoord2f(w_aspect, h_aspect); glVertex2i(1, 1);  
        glTexCoord2f(0.0f, h_aspect);     glVertex2i(-1, 1);   
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
