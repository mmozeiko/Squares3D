#include <GL/glfw.h>
#include <GL/glext.h>

#include "font.h"
#include "file.h"
#include "xml.h"
#include "video.h"
#include "vmath.h"

static const Vector ShadowColor(0.1f, 0.1f, 0.1f);

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

struct Char
{
    int id;
    int x;
    int y;
    int width;
    int height;
    int xoffset;
    int yoffset;
    int xadvance;
};

Font::Font(const string& filename) : m_texture(0)
{
    clog << "Loading font '" << filename << "'..." << endl;

    XMLnode xml;
    File::Reader in("/data/font/" + filename + ".fnt");
    if (!in.is_open())
    {
        throw Exception("Font file '" + filename + "'.fnt not found");  
    }
    xml.load(in);
    in.close();

    map<int, Char> chars;

    m_count = -1;
    float texW, texH;
    float size;

    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "info")
        {
            size = node.getAttribute<float>("size");
        } 
        else if (node.name == "common")
        {
            m_height = node.getAttribute<int>("lineHeight");
            texW = node.getAttribute<float>("scaleW");
            texH = node.getAttribute<float>("scaleH");
            if (node.getAttribute<int>("pages") != 1 ||
                node.getAttribute<int>("packed") != 0)
            {
                throw Exception("Invalid font file - '" + filename + "'. Font pages or packing not supported!");
            }
        }
        else if (node.name == "page")
        {
            if (m_texture != 0)
            {
                throw Exception("Only one paged fonts supported");
            }

            File::Reader file("/data/font/" + node.getAttribute("file"));
            if (!file.is_open())
            {
                throw Exception("Font page '" + node.getAttribute("file") + "' not found");
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
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else if (node.name == "char")
        {
            Char c;
            c.id = node.getAttribute<int>("id");
            c.x = node.getAttribute<int>("x");
            c.y = node.getAttribute<int>("y");
            c.width = node.getAttribute<int>("width");
            c.height = node.getAttribute<int>("height");
            c.xoffset = node.getAttribute<int>("xoffset");
            c.yoffset = node.getAttribute<int>("yoffset");
            c.xadvance = node.getAttribute<int>("xadvance");

            if (c.id > m_count)
            {
                m_count = c.id;
            }

            chars[c.id] = c;
        }
    }

    m_listbase = glGenLists(m_count);

    for (int ch = 0; ch < m_count; ch++)
    {
        const Char& c = chars[ch];
        glNewList(m_listbase + ch-1, GL_COMPILE);

        if (foundInMap(chars, c.id))
        {
            float u1 = c.x / texW;
            float v1 = 1.0f - c.y / texH;
            float u2 = (c.x + c.width) / texW;
            float v2 = 1.0f - (c.y + c.height) / texH;

            float x1 = static_cast<float>(c.xoffset);
            float y1 = static_cast<float>(m_height - c.yoffset);
            float x2 = static_cast<float>(c.xoffset + c.width);
            float y2 = static_cast<float>(m_height - (c.yoffset + c.height));
            
            glBegin(GL_QUADS);
                glTexCoord2f(u1, v2); glVertex2f(x1, y2);
                glTexCoord2f(u2, v2); glVertex2f(x2, y2);
                glTexCoord2f(u2, v1); glVertex2f(x2, y1);
                glTexCoord2f(u1, v1); glVertex2f(x1, y1);
            glEnd();
      
            glTranslatef(static_cast<float>(c.xadvance - c.xoffset), 0.0, 0.0);
        }
        else
        {
            glTranslatef(size, 0.0, 0.0);
        }

        glEndList();
    }
    
    m_widths.resize(m_count);

    for (int i=0; i<m_count; i++)
    {
        m_widths[i] = chars[i].width;
    }
}

Font::~Font()
{
    glDeleteLists(m_listbase, m_count);
    glDeleteTextures(1, &m_texture);
}

void Font::begin(bool shadowed, float shadowWidth) const
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
    gluOrtho2D(viewport[0], viewport[2], viewport[1], viewport[3]);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glListBase(m_listbase);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, m_texture);
    glEnable(GL_TEXTURE_2D);

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
                glColor3fv(ShadowColor.v);
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
    glDepthMask(GL_TRUE);
}

int Font::getWidth(const wstring& text) const
{
    int result = 0;
    for (size_t i=0; i<text.size(); i++)
    {
        if (text[i] < m_widths.size())
        {
            result += m_widths[text[i]];
        }
    }
    return result;
}

int Font::getHeight() const
{
    return m_height;
}
