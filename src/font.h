#ifndef __FONT_H__
#define __FONT_H__

#include "common.h"
#include "vmath.h"

class Font : public NoCopy
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

    bool hasChar(int ch) const;
    int getWidth(const string& text) const;
    int getHeight() const;
    int getHeight(const string& text) const;

    void begin(bool shadowed = true, const Vector& shadow = Vector(0.1f, 0.1f, 0.1f), float shadowWidth = 1.5f) const;
    void render(const string& text, AlignType align = Align_Left) const;
    void end() const;

    void begin2() const;

private:
    Font(const string& filename);
    ~Font();

    unsigned int m_listbase;
    unsigned int m_texture;

    int       m_count;
    int       m_height;
    IntVector m_widths;
    
    mutable Vector    m_shadow;
    mutable bool      m_shadowed;
    mutable float     m_shadowWidth;
    
    void renderPlain(const string& text) const;
};

#endif
