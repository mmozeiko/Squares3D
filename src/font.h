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

    static const Font* get(const string& name);
    static void unload();

    int getWidth(const wstring& text) const;
    int getHeight() const;

    void begin(bool shadowed = true, float shadowWidth = 1.5f) const;
    void render(const wstring& text, AlignType align = Align_Left) const;
    void end() const;

    unsigned int m_texture;

private:
    Font(const string& filename);
    ~Font();

    unsigned int m_listbase;

    int       m_count;
    int       m_height;
    IntVector m_widths;
    
    mutable bool      m_shadowed;
    mutable float     m_shadowWidth;
    
    void renderPlain(const wstring& text) const;
};

#endif
