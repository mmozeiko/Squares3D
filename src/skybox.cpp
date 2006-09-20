#include <GL/glfw.h>

#include "skybox.h"
#include "video.h"
#include "texture.h"

SkyBox::SkyBox()
{
    static const string faces[] = {
        "_posx",
        "_negx",
        "_posy",
        "_negy",
        "_posz",
        "_negz",
    };
    for (int i=0; i<6; i++)
    {
        m_texture[i] = Video::instance->loadTexture("cubemap/skybox_1" + faces[i], false);
        m_texture[i]->setWrap(Texture::ClampToEdge);
        m_texture[i]->setFilter(Texture::Bilinear);
    }
}

void SkyBox::render() const
{
    Matrix modelview;
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview.m);

    glPushMatrix();

    modelview.m30 = modelview.m31 = modelview.m32 = 0.0f;
    glLoadMatrixf(modelview.m);

    glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    static const float vertices[][3] = {
        { -0.5f, -0.5f, -0.5f }, // 0
        {  0.5f, -0.5f, -0.5f }, // 1
        {  0.5f, -0.5f,  0.5f }, // 2
        { -0.5f, -0.5f,  0.5f }, // 3

        { -0.5f, 0.5f, -0.5f },  // 4
        {  0.5f, 0.5f, -0.5f },  // 5
        {  0.5f, 0.5f,  0.5f },  // 6
        { -0.5f, 0.5f,  0.5f },  // 7
    };

    static const int faces[][4] = {
        { 1, 2, 6, 5 }, // right
        { 3, 0, 4, 7 }, // left
        { 4, 5, 6, 7 }, // up
        { 3, 2, 1, 0 }, // bottom
        { 0, 1, 5, 4 }, // back
        { 2, 3, 7, 6 }, // front
    };
    
    static const float uv[][2] = {
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 0.0f },
        { 0.0f, 0.0f },
    };
            
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    for (int i = 0; i < 6; i++)
    {
        m_texture[i]->begin();
        glBegin(GL_QUADS);
        for (int k = 0; k < 4; k++)
        {
            glTexCoord2fv(uv[k]);
            glVertex3fv(vertices[faces[i][k]]);
        }
        glEnd();
        m_texture[i]->end();
    }

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glPopAttrib();

    glPopMatrix();
}
