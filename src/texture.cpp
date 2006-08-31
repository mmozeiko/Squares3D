#include <GL/glfw.h>
#include <GL/glext.h>

#include "texture.h"
#include "file.h"
#include "vmath.h"

Texture::Texture(unsigned int type, const FilterType filter, const WrapType wrap) : m_type(type)
{
    glGenTextures(1, &m_handle);
    glBindTexture(m_type, m_handle);

    setFilter(filter);
    setWrap(wrap);
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_handle);
}

void Texture::setFilter(const FilterType filter)
{
    switch (filter)
    {
    case None:
        glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    case Bilinear:
        glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    case Trilinear:
        glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    }
}

void Texture::setWrap(const WrapType wrap)
{
    switch (wrap)
    {
    case Repeat:
        glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;
    case Clamp:
        glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_CLAMP);
        break;
    case ClampToEdge:
        glTexParameteri(m_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(m_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;
    }
}

void Texture::begin() const
{
    glEnable(m_type);
    glBindTexture(m_type, m_handle);
}

void Texture::end() const
{
    glDisable(m_type);
}

void Texture::loadImage(const string& filename, int flags, GLFWimage* image) const
{
    File::Reader file(filename);
    if (!file.is_open())
    {
        throw Exception("Texture '" + filename + "' not found");
    }
    size_t filesize = file.size();

    vector<char> data(filesize);
    file.read(&data[0], filesize);
    file.close();

    glfwReadMemoryImage(&data[0], static_cast<int>(filesize), image, flags);
}

void Texture::upload(GLFWimage* image, unsigned int target) const
{
    if (target == GL_TEXTURE_2D)
    {
        glfwLoadTextureImage2D(image, GLFW_BUILD_MIPMAPS_BIT);
    }
    else
    {
        glTexImage2D(target, 0, image->Format, image->Width, image->Height, 0, image->Format, GL_UNSIGNED_BYTE, image->Data);
    }
}

Texture2D::Texture2D(const string& name) : Texture(GL_TEXTURE_2D, Trilinear, Repeat)
{
    GLFWimage image;
    Texture::loadImage("/data/textures/" + name, 0, &image);
    Texture::upload(&image);
    glfwFreeImage(&image);
}

TextureCube::TextureCube(const string& name) : Texture(GL_TEXTURE_CUBE_MAP_ARB, Bilinear, ClampToEdge)
{
    static const pair<int, string> faces[] = {
        make_pair(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, "_posx.tga"),
        make_pair(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, "_negx.tga"),
        make_pair(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, "_posy.tga"),
        make_pair(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, "_negy.tga"),
        make_pair(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, "_posz.tga"),
        make_pair(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, "_negz.tga"),
    };

    for (int i=0; i<sizeOfArray(faces); i++)
    {
        GLFWimage image;
        Texture::loadImage("/data/textures/cubemap/" + name + faces[i].second, GLFW_NO_RESCALE_BIT, &image);
        Texture::upload(&image, faces[i].first);
        glfwFreeImage(&image);
    }

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

    glTexGenfv(GL_S, GL_OBJECT_PLANE, Vector::X.v);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, Vector::Y.v);
    glTexGenfv(GL_R, GL_OBJECT_PLANE, Vector::Z.v);
}

void TextureCube::begin() const
{
    Texture::begin();
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glDisable(GL_LIGHTING);
}

void TextureCube::end() const
{
    glEnable(GL_LIGHTING);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    Texture::end();
}
