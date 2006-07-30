#include "font.h"
#include "file.h"

#include <GL/glfw.h>
#include <GL/glext.h>

struct BFFheader
{
    unsigned short id;
    unsigned int width;
    unsigned int height;
    unsigned int cell_width;
    unsigned int cell_height;
    unsigned char bpp;
    unsigned char base;
    unsigned char char_width[256];
};

Font::Font(const string& filename)
{
    File::Reader font("/data/font/" + filename);
    if (!font.is_open())
    {
        throw Exception("Font file '" + filename + "' not found");
    }
    char buf[20+256];
    
    if (font.read(buf, sizeof(buf)) != sizeof(buf))
    {
        throw Exception("Invalid font file");
    }
    
    BFFheader header;
    header.id =          *reinterpret_cast<unsigned short*>(&buf[0]);
    header.width =       *reinterpret_cast<unsigned int*>(&buf[2]);
    header.height =      *reinterpret_cast<unsigned int*>(&buf[6]);
    header.cell_width =  *reinterpret_cast<unsigned int*>(&buf[10]);
    header.cell_height = *reinterpret_cast<unsigned int*>(&buf[14]);
    header.bpp =         *reinterpret_cast<unsigned char*>(&buf[18]);
    header.base =        *reinterpret_cast<unsigned char*>(&buf[19]);
    std::copy(buf+20, buf+20+256, header.char_width);

    if (header.id != 0xF2BF || (header.bpp!=8 && header.bpp!=24 && header.bpp!=32))
    {
        throw Exception("Invalid font file header");
    }

    int format;
    switch (header.bpp)
    {
    case 8: format = GL_LUMINANCE; break;
    case 24: format = GL_RGB; break;
    case 32: format = GL_RGBA; break;
    }

    vector<char> data(header.width * header.height * header.bpp/8);
    if (font.read(&data[0], data.size()) != data.size())
    {
        throw Exception("Invalid font file data");
    }
    font.close();

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE_SGIS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE_SGIS);
    glTexImage2D(GL_TEXTURE_2D, 0, format, header.width, header.height, 0, format, GL_UNSIGNED_BYTE, &data[0]);

    m_listbase = glGenLists(256);

    unsigned int maxchar = header.base + (header.width * header.height) / (header.cell_width * header.cell_height);
    int row_pitch = header.width / header.cell_width;

    float col_factor = static_cast<float>(header.cell_width) / header.width;
    float row_factor = static_cast<float>(header.cell_height) / header.height;

    float w = static_cast<float>(header.cell_width);
    float h = static_cast<float>(header.cell_height);

    for (unsigned int ch=0; ch<256; ch++)
    {
        glNewList(m_listbase + ch, GL_COMPILE);

        if (ch>=header.base && ch<=maxchar)
        {
            int idx = ch-header.base;
            int row = idx / row_pitch;
            int col = idx - row*row_pitch;

            float u1 = col * col_factor;
            float v1 = row * row_factor;
            float u2 = u1 + col_factor;
            float v2 = v1 + row_factor;

            glPushMatrix();
            glBegin(GL_QUADS);
                glTexCoord2f(u1, v1); glVertex2f(0, 0);
                glTexCoord2f(u1, v2); glVertex2f(0, h);
                glTexCoord2f(u2, v2); glVertex2f(w, h);
                glTexCoord2f(u2, v1); glVertex2f(w, 0);
            glEnd();
            glPopMatrix();
      
            glTranslatef(header.char_width[ch], 0.0, 0.0);
        }

        glEndList();
    }

    m_bpp = header.bpp;
    m_height = header.cell_height;
    for (int i=0; i<256; i++)
    {
        m_widths[i] = header.char_width[i];
    }
}

Font::~Font()
{
    glDeleteTextures(1, &m_texture);
    glDeleteLists(m_listbase, 256);
}

void Font::begin() const
{
    glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT);

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(viewport[0], viewport[2], viewport[3], viewport[1]);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    if (m_bpp == 8)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
    }
    else if (m_bpp == 32)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}

void Font::render(const string& text) const
{
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glListBase(m_listbase);
    glCallLists(static_cast<unsigned int>(text.size()), GL_UNSIGNED_BYTE, text.c_str());
    glPopMatrix();
}

void Font::end() const
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glDepthMask(GL_TRUE);
}

IntPair Font::getSize(const string& text) const
{
    IntPair result(0, m_height);
    for (size_t i=0; i<text.size(); i++)
    {
        result.first += m_widths[text[i]];
    }
    return result;
}

