#ifndef __FONT_H__
#define __FONT_H__

#include "common.h"

class Font : NoCopy
{
public:
    enum AlignType
    {
        Align_Left,
        Align_Center,
        Align_Right,
    };

    Font(const string& filename);
    ~Font();

    IntPair getSize(const string& text) const;

    void begin(AlignType align = Align_Left, bool shadowed = true, float shadowWidth = 1.5f) const;
    void render(const string& text) const;
    void end() const;

private:
    unsigned int m_texture;
    unsigned int m_listbase;

    int  m_bpp;
    int  m_height;
    int  m_widths[256];
    
    mutable bool      m_shadowed;
    mutable AlignType m_align;
    mutable float     m_shadowWidth;
    
    void renderPlain(const string& text) const;
};

#endif
