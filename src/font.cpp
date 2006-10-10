#include <GL/glfw.h>
#include <GL/glext.h>

#include "font.h"
#include "file.h"
#include "vmath.h"

typedef map<string, const Font*> FontMap;

FontMap fonts;

const Font* Font::get(const string& name)
{
    FontMap::const_iterator iter = fonts.find(name);
    if (iter != fonts.end())
    {
        return iter->second;
    }
    return fonts.insert(make_pair(name, new Font(name))).first->second;
}

void Font::unload()
{
    for each_(FontMap, fonts, iter)
    {
        delete iter->second;
    }
    fonts.clear();
}

#pragma pack ( push )
#pragma pack ( 1 )

struct Head
{
    char fnt[4];
    short size;
    short texW;
    short texH;
    short height;
    short count;
    short maxid;
};

struct Char
{
    short id;
    short x;
    short y;
    char width;
    char height;
    char xoffset;
    char yoffset;
    char xadvance;
};
#pragma pack ( pop )

Font::Font(const string& filename) : m_texture(0)
{
    clog << "Loading font '" << filename << "'..." << endl;

    // loading font description
    File::Reader in("/data/font/" + filename + ".font");
    if (!in.is_open())
    {
        throw Exception("Font file not found");  
    }
    
    Head head;
    in.read(&head, sizeof(Head));
    if (string(head.fnt+0, head.fnt+4) != "FONT")
    {
        throw Exception("Invalid font file");
    }
    m_count = head.maxid;
    m_height = head.height;

    vector<Char> chars(head.count);
    in.read(&chars[0], sizeof(Char)*head.count);
    in.close();

    // loading texture
    File::Reader file("/data/font/" + filename + "_00.tga");
    if (!file.is_open())
    {
        throw Exception("Font texture '" + filename + "_00.tga' not found");
    }
    size_t filesize = file.size();
    vector<char> data(filesize);
    file.read(&data[0], filesize);
    file.close();

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    GLFWimage image;
    glfwReadMemoryImage(&data[0], static_cast<int>(filesize), &image, GLFW_NO_RESCALE_BIT);
    if (image.BytesPerPixel == 1)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, image.Width, image.Height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, image.Data);
    }
    else if (image.BytesPerPixel == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.Width, image.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.Data);
    }
    else
    {
        assert(false);
    }

    glfwFreeImage(&image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    
    
    m_listbase = glGenLists(m_count);
    m_widths.resize(m_count);

    int idx = 0;
    int pos = 0;
    while (idx < m_count)
    {
        while (idx != chars[pos].id)
        {
            m_widths[idx] = head.size;
            glNewList(m_listbase + idx++, GL_COMPILE);
            glTranslatef(head.size, 0.0f, 0.0f);
            glEndList();
        }
        
        const Char& c = chars[pos++];
        m_widths[idx] = c.xadvance; // - c.xoffset;
        glNewList(m_listbase + idx++, GL_COMPILE);

        float u1 = static_cast<float>(c.x) / static_cast<float>(head.texW);
        float v1 = 1.0f - static_cast<float>(c.y) / head.texH;
        float u2 = static_cast<float>(c.x + c.width) / static_cast<float>(head.texW);
        float v2 = 1.0f - static_cast<float>(c.y + c.height) / head.texH;

        float x1 = static_cast<float>(c.xoffset);
        float y1 = static_cast<float>(m_height - c.yoffset);
        float x2 = static_cast<float>(c.xoffset + c.width);
        float y2 = static_cast<float>(m_height - (c.yoffset + c.height));
        
        glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(u1, v1); glVertex2f(x1, y1);
            glTexCoord2f(u1, v2); glVertex2f(x1, y2);
            glTexCoord2f(u2, v1); glVertex2f(x2, y1);
            glTexCoord2f(u2, v2); glVertex2f(x2, y2);
        glEnd();
  
        glTranslatef(static_cast<float>(c.xadvance), 0.0f, 0.0f); //  - c.xoffset

        glEndList();
    }
}

Font::~Font()
{
    glDeleteLists(m_listbase, m_count);
    glDeleteTextures(1, &m_texture);
}

bool Font::hasChar(int ch) const
{
    return ch>=0 && ch<=static_cast<int>(m_widths.size()) && m_widths[ch]>0;
}

void Font::begin(bool shadowed, const Vector& shadow, float shadowWidth) const
{
    glPushAttrib(
        GL_COLOR_BUFFER_BIT |
        GL_CURRENT_BIT |
        GL_DEPTH_BUFFER_BIT |
        GL_ENABLE_BIT |
        GL_LIGHTING_BIT |
        GL_LIST_BIT |
        GL_TRANSFORM_BIT |
        GL_POLYGON_BIT);

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(viewport[0], viewport[2], viewport[1], viewport[3], -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glListBase(m_listbase);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, m_texture);

    m_shadow = shadow;
    m_shadowed = shadowed;
    m_shadowWidth = shadowWidth;
}

void Font::renderPlain(const wstring& text) const
{
    glPushMatrix();
    glCallLists(static_cast<unsigned int>(text.size()), GL_UNSIGNED_SHORT, text.c_str());
    glPopMatrix();
}

void Font::render(const wstring& text, AlignType align) const
{
    Vector shadowColor(m_shadow);
    //clog << shadowColor << endl;
    Vector forAlpha;
    glGetFloatv(GL_CURRENT_COLOR, forAlpha.v);
    shadowColor.w = forAlpha.w;
    //

    size_t pos, begin = 0;
    wstring line;
    float linePos = 0.0f;
    int width;
    while (begin < text.size())
    {
        pos = text.find(L'\n', begin);
        if (pos == wstring::npos)
        {
            pos = text.size();
        }

        line = text.substr(begin, pos - begin);

        begin = pos + 1;

        glPushMatrix();

        switch (align)
        {
        case Align_Left:
            break;
        case Align_Center:
            width = getWidth(line);
            glTranslatef(static_cast<float>(-width/2), 0.0f, 0.0f);
            break;
        case Align_Right:
            width = getWidth(line);
            glTranslatef(static_cast<float>(-width), 0.0f, 0.0f);
            break;
        default:
            assert(false);
        };
        glTranslatef(0.0f, linePos, 0.0f);       

        if (m_shadowed)
        {
            glPushAttrib(GL_CURRENT_BIT);
            glPushMatrix();
                glColor4fv(shadowColor.v);
                glTranslatef(m_shadowWidth, - m_shadowWidth, 0.0f);
                renderPlain(line);
            glPopMatrix();
            glPopAttrib();
        }

        renderPlain(line);
        glPopMatrix();
        linePos -= m_height;
    }

}

void Font::end() const
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
}

int Font::getWidth(const wstring& text) const
{
    int maxWidth = 0;
    int width = 0;
    for (size_t i=0; i<text.size(); i++)
    {
        if (text[i] < m_widths.size())
        {
            if (text[i] == L'\n')
            {
                if (width > maxWidth)
                {
                    maxWidth = width;
                }
                width = 0;
            }
            width += m_widths[text[i]];
        }
    }
    if (width > maxWidth)
    {
        maxWidth = width;
    }
    return maxWidth;
}

int Font::getHeight() const
{
    return m_height;
}
