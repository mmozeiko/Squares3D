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

    Texture(const string& name, bool mipmaps = false);
    ~Texture();

    void setFilter(const FilterType filter);
    void setWrap(const WrapType wrap);

    void begin() const;
    void end() const;

protected:
    void loadImage(const string& filename, int flags, GLFWimage* image) const;
    void upload(GLFWimage* image, bool mipmaps) const;

private:
    unsigned int m_handle;
};

#endif
