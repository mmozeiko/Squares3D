#include "framebuffer.h"
#include "video.h"

FrameBuffer::FrameBuffer() :
    m_size(0), m_colorTex(0), m_shadowTex(0),
    m_frameBuffer(0), m_renderBuffer(0)
{
}

FrameBuffer::~FrameBuffer()
{
    destroy();
}

void FrameBuffer::create(unsigned int size)
{
    m_size = size;

    destroy();
    Video::glGenFramebuffersEXT(1, &m_frameBuffer);
    Video::glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frameBuffer);
}

void FrameBuffer::destroy()
{
    if (m_colorTex != 0) 
    {
        glDeleteTextures(1, &m_colorTex);
        m_colorTex = 0;
    }

    if (m_shadowTex != 0)
    {
        glDeleteTextures(1, &m_shadowTex);
        m_shadowTex = 0;
    }
    
    if (m_frameBuffer != 0)
    {
        Video::glDeleteFramebuffersEXT(1, &m_frameBuffer);
        m_frameBuffer = 0;
    }
    
    if (m_renderBuffer != 0)
    {
        Video::glDeleteRenderbuffersEXT(1, &m_renderBuffer);
        m_renderBuffer = 0;
    }
}

bool FrameBuffer::isValid() const
{
    GLenum status = Video::glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

    Video::glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    Video::glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

    return (status==GL_FRAMEBUFFER_COMPLETE_EXT);
}

void FrameBuffer::bind() const
{
    Video::glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frameBuffer);
}

void FrameBuffer::unbind() const
{
    Video::glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

unsigned int FrameBuffer::attachColorTex()
{
    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_size, m_size, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    Video::glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_colorTex, 0);

    return m_colorTex;
}

unsigned int FrameBuffer::attachDepthTex()
{
    Video::glGenRenderbuffersEXT(1, &m_renderBuffer);
    Video::glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_renderBuffer);
    Video::glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, m_size, m_size);
    Video::glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_renderBuffer);

    // create the texture we'll use for the shadow map
    glGenTextures(1, &m_shadowTex);
    glBindTexture(GL_TEXTURE_2D, m_shadowTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_size, m_size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    Video::glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_shadowTex, 0);

    return m_shadowTex;
}
