#include <GL/glfw.h>

#include "skybox.h"
#include "video.h"
#include "vmath.h"
#include "file.h"

static void loadTex(const string& name, GLFWimage* image)
{
    string filename = "/data/textures/" + name;
    File::Reader file(filename);
    if (!file.is_open())
    {
        throw Exception("Texture '" + name + "' not found");
    }
    size_t filesize = file.size();

    vector<char> data(filesize);
    file.read(&data[0], filesize);
    file.close();

    glEnable(GL_TEXTURE_2D);

    glfwReadMemoryImage(&data[0], static_cast<int>(filesize), image, GLFW_NO_RESCALE_BIT);
}

SkyBox::SkyBox(Video* video, const string& name)
{
    static const pair<int, string> texs[] = {
        make_pair(GL_TEXTURE_CUBE_MAP_POSITIVE_X, name + "posx.tga"),
        make_pair(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, name + "negx.tga"),
        make_pair(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, name + "posy.tga"),
        make_pair(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, name + "negy.tga"),
        make_pair(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, name + "posz.tga"),
        make_pair(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, name + "negz.tga"),
    };

    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, cubemap);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLFWimage img;

    static const Vector X(1.0f, 0.0f, 0.0f, 0.0f);
    static const Vector Y(0.0f, 1.0f, 0.0f, 0.0f);
    static const Vector Z(0.0f, 0.0f, 1.0f, 0.0f);

    for (int i=0; i<6; i++)
    {
        loadTex(texs[i].second, &img);
        glTexImage2D(texs[i].first, 0, img.Format, img.Width, img.Height, 0, img.Format, GL_UNSIGNED_BYTE, img.Data);
        glfwFreeImage(&img);

        glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGenf(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

        glTexGenfv(GL_S, GL_OBJECT_PLANE, X.v);
        glTexGenfv(GL_T, GL_OBJECT_PLANE, Y.v);
        glTexGenfv(GL_R, GL_OBJECT_PLANE, Z.v);
    }
}

SkyBox::~SkyBox()
{
    glDeleteTextures(1, &cubemap);
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

    glEnable(GL_TEXTURE_CUBE_MAP_ARB);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, cubemap);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);

    video->renderCube();

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDisable(GL_TEXTURE_CUBE_MAP_ARB);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glPopMatrix();
}

