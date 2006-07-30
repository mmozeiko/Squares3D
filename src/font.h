#ifndef __FONT_H__
#define __FONT_H__

#include "common.h"

class Font : NoCopy
{
public:
    Font(const string& filename);
    ~Font();

    IntPair getSize(const string& text) const;

    void begin() const;
    void render(const string& text) const;
    void end() const;

private:
    unsigned int m_texture;
    unsigned int m_listbase;

    int m_bpp;
    int m_height;
    int m_widths[256];
};

#endif
