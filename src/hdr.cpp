#include "hdr.h"
#include "shader.h"
#include "framebuffer.h"
#include "config.h"

HDR::HDR() :
    m_downsample(NULL),
    m_blur(NULL),
    m_final(NULL),
    m_sourceTex(0),
    m_fboSource(NULL),
    m_w(0.0f),
    m_h(0.0f),
    m_valid(false)
{
    for (int i=0; i<hdr_levels; i++)
    {
        m_downsampledTex[i][0] = m_downsampledTex[i][1] = 0;
        m_fboDownsampled[i] = NULL;
    }
}

void HDR::updateFromLevel(float eps, float exp, const Vector& mul)
{
    if (!m_valid)
    {
        return;
    }

    m_downsample->begin();
    m_downsample->setFloat1("hdr_eps", eps);
    m_downsample->setFloat4("hdr_mul", mul);
    m_downsample->end();

    m_final->begin();
    m_final->setFloat1("hdr_exposure", exp);
    m_final->end();
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
        m_blur = new Shader("hdr_blur");
        m_blur->setInt1("tex_source", 0);
        m_blur->end();
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

    if (m_downsample->valid()==false || m_blur->valid()==false || m_final->valid()==false)
    {
        Config::instance->m_video.use_hdr = false;
        return;
    }
    
    m_fboSource = new FrameBuffer();

    for (int i=0; i<hdr_levels; i++)
    {
        m_fboDownsampled[i] = new FrameBuffer();
    }

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
    m_fboSource->attachRBufDepth();
    if (!m_fboSource->isValid())
    {
        Config::instance->m_video.use_hdr = false;
        return;
    }

    for (int i=0; i<hdr_levels; i++)
    {
        m_fboDownsampled[i]->create(256 >> i, 256 >> i);
        m_downsampledTex[i][0] = m_fboDownsampled[i]->attachColorTex(true);
        m_downsampledTex[i][1] = m_fboDownsampled[i]->attachColorTex1(true);
    
        if (!m_fboDownsampled[i]->isValid() )
        {
            Config::instance->m_video.use_hdr = false;
            return;
        }
    }

    m_valid = true;
}

HDR::~HDR()
{
    if (m_downsample != NULL) delete m_downsample;
    if (m_blur != NULL) delete m_blur;
    if (m_final != NULL) delete m_final;
    if (m_fboSource != NULL) delete m_fboSource;
    for (int i=0; i<hdr_levels; i++)
    {
        if (m_fboDownsampled[i] != NULL) delete m_fboDownsampled[i];
    }
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

    GLint m_view[4];
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

    // downsample from source texture to smaller textures
    glBindTexture(GL_TEXTURE_2D, m_sourceTex);
  
    m_downsample->begin();
    for (int i=0; i<hdr_levels; i++)
    {
        glViewport(0, 0, 256 >> i, 256 >> i);
        m_fboDownsampled[i]->bind();
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
            glTexCoord2f(m_w, 0.0f);  glVertex2f(m_w, 0.0f);
            glTexCoord2f(m_w, m_h);   glVertex2f(m_w, m_h);
            glTexCoord2f(0.0f, m_h);  glVertex2f(0.0f, m_h);
        glEnd();
        m_fboDownsampled[i]->unbind();
    }
    m_downsample->end();


    // blur small textures
    m_blur->begin();
    for (int i=0; i<hdr_levels; i++)
    {
        glViewport(0, 0, 256 >> i, 256 >> i);
        m_fboDownsampled[i]->bind();

        // horizontal blur
        m_blur->setFloat4("tex_offset", Vector(1.0f/(256 >> i), 0.0f, 0.0f, 0.0f));
        glBindTexture(GL_TEXTURE_2D, m_downsampledTex[i][0]);
        glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
            glTexCoord2f(m_w, 0.0f); glVertex2f(m_w, 0.0f);
            glTexCoord2f(m_w, m_h); glVertex2f(m_w, m_h);
            glTexCoord2f(0.0f, m_h); glVertex2f(0.0f, m_h);
        glEnd();

        // vertical blur
        m_blur->setFloat4("tex_offset", Vector(0.0f, 1.0f/(256 >> i), 0.0f, 0.0f));
        glBindTexture(GL_TEXTURE_2D, m_downsampledTex[i][1]);
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
            glTexCoord2f(m_w, 0.0f); glVertex2f(1.0f, 0.0f);
            glTexCoord2f(m_w, m_h); glVertex2f(1.0f, 1.0f);
            glTexCoord2f(0.0f, m_h); glVertex2f(0.0f, 1.0f);
        glEnd();

        /* for DEBUG!!
        {glBindTexture(GL_TEXTURE_2D, m_downsampledTex[i][1]);
        vector<char> data((256 >> i)*(256 >> i)*3);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);
        FILE* f = fopen(("xxx_" + cast<string>(256 >> i) + ".raw").c_str(), "wb");
        fwrite(&data[0], data.size(), 1, f);
        fclose(f);}*/

        m_fboDownsampled[i]->unbind();
    }
    m_blur->end();



    // draw source texture and all smaller textures together

    glViewport(m_view[0], m_view[1], m_view[2], m_view[3]);
    glDrawBuffer(draw_buffer);


    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, m_sourceTex);
    for (int i=0; i<hdr_levels; i++)
    {
        glActiveTextureARB(GL_TEXTURE1_ARB + i);
        glBindTexture(GL_TEXTURE_2D, m_downsampledTex[i][0]);
        glEnable(GL_TEXTURE_2D);
    }

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

    for (int i=0; i<hdr_levels; i++)
    {
        glActiveTextureARB(GL_TEXTURE1_ARB + i);
        glDisable(GL_TEXTURE_2D);
    }
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
