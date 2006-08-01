#include <GL/glfw.h>

#include "skybox.h"
#include "video.h"
#include "texture.h"

SkyBox::SkyBox(Video* video)
{
    m_texture = video->loadCubeMap("skybox");
}

void SkyBox::render(const Video* video) const
{
    Matrix modelview;
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview.m);
    modelview.m30 = modelview.m31 = modelview.m32 = 0.0f;

    glPushMatrix();
    glLoadMatrixf(modelview.m);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    m_texture->begin();
    video->renderCube();
    m_texture->end();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glPopMatrix();
}
