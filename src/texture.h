#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <GL/glfw.h>

#include "common.h"

namespace File
{
    class Reader;
}

class Texture
{
public:
    enum FilterType { None, Bilinear, Trilinear };
    enum WrapType { Repeat, Clamp, ClampToEdge };

    Texture(unsigned int type, const FilterType filter, const WrapType wrap);
    virtual ~Texture();

    virtual void begin() const;
    virtual void end() const;

protected:
    void loadImage(const string& filename, int flags, GLFWimage* image) const;
    void upload(GLFWimage* image, unsigned int target = GL_TEXTURE_2D) const;

private:
    unsigned int m_type;
    unsigned int m_handle;

    void setFilter(const FilterType filter);
    void setWrap(const WrapType wrap);
};

class Texture2D : public Texture
{
public:
    Texture2D(const string& name);
};

class TextureCube : public Texture
{
public:
    TextureCube(const string& name);

    void begin() const;
    void end() const;
};

#endif
