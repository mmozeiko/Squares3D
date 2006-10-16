#include <GL/glfw.h>

#include "menu_submenu.h"
#include "menu_entries.h"
#include "menu_entries.h"
#include "menu.h"

#include "video.h"
#include "input.h"
#include "font.h"
#include "colors.h"
#include "sound.h"

void Submenu::onChar(int ch)
{
    m_entries[m_activeEntry]->onChar(ch);
}

void Submenu::control(int key)
{
    int b;
    do
    {
        b = Input::instance->popButton();
    }
    while (Input::instance->popButton() != -1);
    
    //get mouse position
    const Mouse& mouse = Input::instance->mouse();
    int videoHeight = Video::instance->getResolution().second;
    Vector mousePos = Vector(static_cast<float>(mouse.x), 0, static_cast<float>(videoHeight - mouse.y));

    //adjust active entry depending on up/down keys
    if ((key == GLFW_KEY_DOWN) || (key == GLFW_KEY_UP))
    {
        activateNextEntry(key == GLFW_KEY_DOWN);
        m_menu->m_sound->play(m_menu->m_soundOver);
    }

    //adjust active entry depending on mouse position
    for (size_t i = 0; i < m_entries.size(); i++)
    {
        Entry* currentEntry = m_entries[i];
        if ( (m_previousMousePos != mousePos)
            && currentEntry->isMouseOver(mousePos)
            && (currentEntry->isEnabled()))
        {
            if (m_activeEntry != i)
            {
                m_menu->m_sound->play(m_menu->m_soundOver);
            }
            m_activeEntry = i;
            break;
        }
    }

    Entry* currentEntry = m_entries[m_activeEntry];

    
    if (b != -1)
    {
        if (currentEntry->isMouseOver(mousePos))
        {
            currentEntry->click(b);
        }
    }
    else if (key != -1)
    {
        currentEntry->click(key);
    }

    m_previousMousePos = mousePos;
}

Submenu::~Submenu()
{
    for each_const(Entries, m_entries, iter)
    {
        delete *iter;
    }
}

void Submenu::addEntry(Entry* entry)
{
    m_entries.push_back(entry);
    m_height += entry->getHeight() * (m_font->getHeight() + 2);
}

void Submenu::center(const Vector& centerPos)
{
    m_centerPos = centerPos;
    Vector upperPos = centerPos;

    upperPos.y += m_height / 2 - m_font->getHeight() / 2;

    m_upper.y = upperPos.y + m_font->getHeight();

    int maxX = 0;
        
    for (size_t i = 0; i < m_entries.size(); i++)
    {      
        Entry* entry = m_entries[i];
        entry->calculateBounds(upperPos, m_font);
        upperPos.y -= entry->getHeight() * (m_font->getHeight() - 2);
        
        int l = entry->getMaxLeftWidth(m_font);
        int r = entry->getMaxRightWidth(m_font);
        if (l > maxX)
        {
            maxX = l;
        }
        if (r > maxX)
        {
            maxX = r;
        }
    }

    for (size_t i=0; i<m_entries.size(); i++)
    {
        m_entries[i]->setXBound(centerPos.x - maxX, centerPos.x + maxX);
    }

    m_upper.x = centerPos.x + maxX; //std::max(maxR, maxL);
    m_lower.x = centerPos.x - maxX; //std::max(maxR, maxL);
    m_lower.y = upperPos.y + m_font->getHeight() - 2;
}

void Submenu::setTitle(const wstring& title, const Vector& position)
{
    m_title = title;
    m_titlePos = position;
}

void Submenu::activateNextEntry(bool moveDown)
{
    //should be used just for key input (up/down used)
    if (!m_entries.empty())
    {
        if (moveDown && (m_activeEntry == (m_entries.size() - 1)))
        {
            m_activeEntry = 0;
        }
        else if (!moveDown && (m_activeEntry == 0))
        {
            m_activeEntry = m_entries.size() - 1;
        }
        else
        {
            moveDown ? m_activeEntry++ : m_activeEntry--;
        }
        
        if (!m_entries[m_activeEntry]->isEnabled())
        {
            activateNextEntry(moveDown);
        }

    }
}

void Submenu::render() const
{
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    Video::instance->renderRoundRect(m_lower, m_upper, static_cast<float>(m_font->getHeight()/2));

    Vector upperPos = m_centerPos;
    upperPos.y += m_height / 2 - m_font->getHeight() / 2;

    for (size_t i = 0; i < m_entries.size(); i++)
    {
        Entry* entry = m_entries[i];

        glPushMatrix();
        glTranslatef(upperPos.x, upperPos.y, upperPos.z);
        if (m_activeEntry == i)
        {
            glColor3fv(Yellow.v);              // active entry
        }
        else
        {
            if (entry->isEnabled())
            {
                glColor3fv(White.v);              // normal entry
            }
            else
            {
                glColor3fv(Grey.v); // disabled entry
            }
        }
        entry->render(m_font);
        glPopMatrix();
        upperPos.y -= entry->getHeight() * (m_font->getHeight() - 2);
    }

    if (!m_title.empty())
    {
        // TODO: move somewhere else
        m_fontBig->begin();
        glPushMatrix();
        glTranslatef(m_titlePos.x, m_titlePos.y, 0);
        glColor3fv(darker(Green, 1.5f).v);
        m_fontBig->render(m_title, Font::Align_Center);
        glPopMatrix();
        m_fontBig->end();
    }
}