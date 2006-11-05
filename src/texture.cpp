#include <GLee.h>
#include "texture.h"
#include "file.h"
#include "vmath.h"
#include "config.h"

Texture::Texture(const string& name, bool mipmaps) : m_size(0)
{
    glGenTextures(1, (GLuint*)&m_handle);
    glBindTexture(GL_TEXTURE_2D, m_handle);

    GLFWimage image;
    loadImage("/data/textures/" + name + ".tga", 0, &image);
    upload(&image, mipmaps);
    m_size = image.Width;
    glfwFreeImage(&image);

    setFilter(Trilinear);
    setWrap(Repeat);

    if (Config::instance->m_video.anisotropy > 0)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1 << Config::instance->m_video.anisotropy);
    }
}

Texture::~Texture()
{
    glDeleteTextures(1, (GLuint*)&m_handle);
}

void Texture::setFilter(const FilterType filter)
{
    switch (filter)
    {
    case None:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    case Bilinear:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    case Trilinear:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    }
}

void Texture::setWrap(const WrapType wrap)
{
    switch (wrap)
    {
    case Repeat:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;
    case Clamp:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        break;
    case ClampToEdge:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;
    }
}

void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, m_handle);
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

void Texture::upload(GLFWimage* image, bool mipmaps) const
{
    glfwLoadTextureImage2D(image, (mipmaps ? GLFW_BUILD_MIPMAPS_BIT : 0));
}

/*
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
}
*/
