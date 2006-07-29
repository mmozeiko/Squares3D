#include "skybox.h"
#include "video.h"
#include "vmath.h"

#include <GL/glfw.h>

SkyBox::SkyBox(Video* video, const string& name)
{
    static const string images[] = {
        name + "posx.tga",
        name + "posz.tga",
        name + "negx.tga",
        name + "negz.tga",
        name + "posy.tga",
        name + "negy.tga",
    };

    for (int i=0; i<6; i++)
    {
        textures[i] = video->loadTexture(images[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

SkyBox::~SkyBox()
{
}

void SkyBox::render(const Video* video) const
{
    Matrix m;
    glGetFloatv(GL_MODELVIEW_MATRIX, m.m);
    m.m30 = m.m31 = m.m32 = 0.0f;

    glPushMatrix();
    glLoadMatrixf(m.m);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    video->applyTexture(textures[0]);

    const float r = 1.0;
    const float t = 1.0;

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-r, -r, -t);
        glTexCoord2f(1, 0); glVertex3f(r, -r, -t);
        glTexCoord2f(1, 1); glVertex3f(r, r, -t);
        glTexCoord2f(0, 1); glVertex3f(-r, r, -t);

    glEnd();

    video->applyTexture(textures[1]);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(t, -r, -r);
        glTexCoord2f(1, 0); glVertex3f(t, -r, r);
        glTexCoord2f(1, 1); glVertex3f(t, r, r);
        glTexCoord2f(0, 1); glVertex3f(t, r, -r);
    glEnd();

    video->applyTexture(textures[2]);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(r, -r, t);
        glTexCoord2f(1, 0); glVertex3f(-r, -r, t);
        glTexCoord2f(1, 1); glVertex3f(-r, r, t);
        glTexCoord2f(0, 1); glVertex3f(r, r, t);
    glEnd();

    video->applyTexture(textures[3]);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-t, -r, r);
        glTexCoord2f(1, 0); glVertex3f(-t, -r, -r);
        glTexCoord2f(1, 1); glVertex3f(-t, r, -r);
        glTexCoord2f(0, 1); glVertex3f(-t, r, r);
    glEnd();

    video->applyTexture(textures[4]);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-r, t, -r);
        glTexCoord2f(1, 0); glVertex3f(r, t, -r);
        glTexCoord2f(1, 1); glVertex3f(r, t, r);
        glTexCoord2f(0, 1); glVertex3f(-r, t, r);
    glEnd();

    video->applyTexture(textures[5]);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-r, -t, r);
        glTexCoord2f(1, 0); glVertex3f(r, -t, r);
        glTexCoord2f(1, 1); glVertex3f(r, -t, -r);
        glTexCoord2f(0, 1); glVertex3f(-r, -t, -r);
    glEnd();

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glPopMatrix();
}

