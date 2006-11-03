#include "framebuffer.h"
#include "video.h"
#include "video_ext.h"

FrameBuffer::FrameBuffer() :
    m_sizeX(0),
    m_sizeY(0),
    m_colorTex(0),
    m_colorTex1(0),
    m_shadowTex(0),
    m_frameBuffer(0)
{
}

FrameBuffer::~FrameBuffer()
{
    destroy();
}

void FrameBuffer::create(unsigned int sizeX, unsigned int sizeY)
{
    m_sizeX = sizeX;
    m_sizeY = sizeY;

    destroy();
    glGenFramebuffersEXT(1, (GLuint*)&m_frameBuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frameBuffer);
}

void FrameBuffer::destroy()
{
    if (m_colorTex != 0) 
    {
        glDeleteTextures(1, (GLuint*)&m_colorTex);
        m_colorTex = 0;
    }

    if (m_colorTex1 != 0) 
    {
        glDeleteTextures(1, (GLuint*)&m_colorTex1);
        m_colorTex1 = 0;
    }

    if (m_shadowTex != 0)
    {
        glDeleteTextures(1, (GLuint*)&m_shadowTex);
        m_shadowTex = 0;
    }
    
    if (m_frameBuffer != 0)
    {
        glDeleteFramebuffersEXT(1, (GLuint*)&m_frameBuffer);
        m_frameBuffer = 0;
    }
}

bool FrameBuffer::isValid() const
{
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    return (status==GL_FRAMEBUFFER_COMPLETE_EXT);
}

void FrameBuffer::bind() const
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frameBuffer);
}

void FrameBuffer::unbind() const
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

unsigned int FrameBuffer::attachColorTex(bool hdr)
{
    glGenTextures(1, (GLuint*)&m_colorTex);
    if (hdr)
    {
        glBindTexture(GL_TEXTURE_2D, m_colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_sizeX, m_sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, m_colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_sizeX, m_sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_colorTex, 0);

    return m_colorTex;
}

unsigned int FrameBuffer::attachColorTex1(bool hdr)
{
    glGenTextures(1, (GLuint*)&m_colorTex1);
    if (hdr)
    {
        glBindTexture(GL_TEXTURE_2D, m_colorTex1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_sizeX, m_sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, m_colorTex1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_sizeX, m_sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, m_colorTex1, 0);

    return m_colorTex1;
}

unsigned int FrameBuffer::attachDepthTex(bool hdr)
{
    // create the texture we'll use for the shadow map
    glGenTextures(1, (GLuint*)&m_shadowTex);
    glBindTexture(GL_TEXTURE_2D, m_shadowTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_sizeX, m_sizeY, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_shadowTex, 0);

    return m_shadowTex;
}
