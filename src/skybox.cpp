#include <GL/glfw.h>

#include "skybox.h"
#include "texture.h"

SkyBox::SkyBox()
{
    static const string facesTex[] = {
        "_posx",
        "_negx",
        "_posy",
        "_negy",
        "_posz",
        "_negz",
    };
    for (int i=0; i<6; i++)
    {
        m_texture[i] = Video::instance->loadTexture("cubemap/skybox_1" + facesTex[i], false);
        m_texture[i]->setWrap(Texture::ClampToEdge);
        m_texture[i]->setFilter(Texture::Bilinear);
    }

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
        { 1, 2, 5, 6 }, // right
        { 3, 0, 7, 4 }, // left
        { 4, 5, 7, 6 }, // up
        { 3, 2, 0, 1 }, // bottom
        { 0, 1, 4, 5 }, // back
        { 2, 3, 6, 7 }, // front
    };
    
    static const float uv[][2] = {
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
    };

    for (int i = 0; i < 6; i++)
    {
        m_faces[i].uv.resize(4);
        m_faces[i].vertexes.resize(4);
        for (int k = 0; k < 4; k++)
        {
            const float* v = vertices[faces[i][k]];
            m_faces[i].uv[k] = UV(uv[k][0], uv[k][1]);
            m_faces[i].vertexes[k] = Vector(v[0], v[1], v[2]);
        }
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
    glDisable(GL_LIGHTING);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glDisableClientState(GL_NORMAL_ARRAY);
    for (int i = 0; i < 6; i++)
    {
        m_texture[i]->bind();
        Video::instance->renderFace(m_faces[i]);
    }
    glEnableClientState(GL_NORMAL_ARRAY);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glPopAttrib();

    glPopMatrix();
}
