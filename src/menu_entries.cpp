#include <GL/glfw.h>

#include "menu_entries.h"
#include "menu_submenu.h"
#include "menu.h"

#include "language.h"
#include "font.h"
#include "input.h"
#include "video.h"
#include "colors.h"
#include "geometry.h"

/*** COLORVALUE ***/

static const pair<wstring, Vector> namesToColors[] = { 
      make_pair(L"Red", Red),
      make_pair(L"Green", Green),
      make_pair(L"Blue", Blue),
      make_pair(L"Black", Black),
      make_pair(L"White", White),
      make_pair(L"Yellow", Yellow),
      make_pair(L"Cyan", Cyan),
      make_pair(L"Magenta", Magenta),
      make_pair(L"Grey", Grey),
      make_pair(L"Pink", Pink)
      };

typedef map<wstring, Vector> Colors;

static Colors colors(namesToColors, namesToColors + sizeOfArray(namesToColors));

ColorValue::ColorValue(const string& id) : Value(id)
{
    for each_const(Colors, colors, iter)
    {
        Value::add(wcast<wstring>(iter->first));        
    }
}

void ColorValue::setCurrent(const Vector& color)
{
    wstring colorName = Language::instance->get(TEXT_CUSTOM);
    bool wasInColorDict = false;
    for each_const(Colors, colors, iter)
    {
        if (iter->second == color)
        {
            colorName = wcast<wstring>(iter->first);
            wasInColorDict = true;
            break;
        }
    }
    if (!wasInColorDict)
    {
        add(colorName);
        colors[colorName] = color;
    }
    for (size_t i = 0; i < m_values.size(); i++)
    {
        if (m_values[i] == colorName)
        {
            m_current = i;
            break;
        }
    }
}

/*** VALUE ***/

wstring Value::getCurrent() const
{
    if (m_values.empty())
    {
        return L"";
    }
    return m_values[m_current];
}

void Value::activateNext(bool forward)
{
    if (!m_values.empty())
    {
        if (forward)
        {
            if (m_current == (m_values.size() - 1))
            {
                m_current = 0;
            }
            else
            {
                m_current++;
            }
        }
        else
        {
            if (m_current == 0)
            {
                m_current = m_values.size() - 1;
            }
            else
            {
                m_current--;
            }
        }

    }
}

int Value::getMaxWidth(const Font* font) const
{
    int maxW = 0;
    for each_const(Values, m_values, value)
    {
        int w = font->getWidth(*value);
        if (w > maxW)
        {
            maxW = w;
        }
    }
    return maxW;
}

/*** ENTRY ***/

bool Entry::isMouseOver(const Vector& mousePos) const
{
    return isPointInRectangle(mousePos, m_lowerLeft, m_upperRight);
}

void Entry::calculateBounds(const Vector& position, const Font* font)
{
    m_lowerLeft = Vector(position.x - font->getWidth(getString())/2.0f, 0, position.y);
    m_upperRight = Vector(position.x + font->getWidth(getString())/2.0f, 0, position.y + font->getHeight());
}

void Entry::setXBound(float minX, float maxX)
{
    m_lowerLeft.x = minX;
    m_upperRight.x = maxX;
}

void Entry::render(const Font* font) const
{
    font->render(getString(), Font::Align_Center);
}

int Entry::getMaxLeftWidth(const Font* font) const
{
    return font->getWidth(m_string)/2;
}

int Entry::getMaxRightWidth(const Font* font) const
{
    return font->getWidth(m_string)/2;
}

/*** COLORENTRY ***/

void ColorEntry::render(const Font* font) const
{
    wstring stringToRender = m_string + L":  ";
    Vector color = colors.find(m_value.getCurrent())->second;
    font->render(stringToRender, Font::Align_Right);

    glColor3f(color.x, color.y, color.z);
    
    const float h = static_cast<float>(font->getHeight());
    const float d = static_cast<float>(font->getWidth(L" ")); 
    
    // uuber magic numbers
    Video::instance->renderRoundRect(
        Vector(d, 3*h/8.0f, 0.0f),
        Vector(d + 100.0f, 3*h/8.0f+h/4.0f, 0.0f),
        h/4.0f);
}

void ColorEntry::click(int button)
{
   if ((button == GLFW_MOUSE_BUTTON_LEFT)
        || (button == GLFW_MOUSE_BUTTON_RIGHT)
        || (button == GLFW_KEY_LEFT)
        || (button == GLFW_KEY_RIGHT)
        || (button == GLFW_KEY_ENTER))
    {
         m_value.activateNext((button == GLFW_MOUSE_BUTTON_LEFT) 
                              || (button == GLFW_KEY_RIGHT)
                              || (button == GLFW_KEY_ENTER));
    }
    m_binding = colors.find(m_value.getCurrent())->second;
}

int ColorEntry::getMaxRightWidth(const Font* font) const
{
    return 100 + font->getWidth(L" ");
}

/*** WRITEABLEENTRY ***/

void WritableEntry::render(const Font* font) const
{
    font->render(m_string + L":  ", Font::Align_Right);

    wstring stringToRender = wcast<wstring>(m_binding);
    if (m_ownerSubmenu->m_entries[m_ownerSubmenu->m_activeEntry] == this)
    {
        if (fmodf(m_timer.read(), 1.0f) > 0.5f)
        {
            stringToRender.push_back('_');
        }
    }
    font->render(stringToRender, Font::Align_Left);
}

void WritableEntry::click(int key)
{ 
    if (key == GLFW_KEY_BACKSPACE)
    {
        if (m_binding.size() > 0)
        {
            m_binding.erase(m_binding.end()-1);
        }
    }
}

void WritableEntry::onChar(int ch)
{ 
    // if we want unicode text, then remove ch<=127, and change m_binding type to wstring
    if (ch<=127 && m_ownerSubmenu->m_font->hasChar(ch) && ((m_string.size() + m_binding.size()) < 15))
    {
        m_binding.push_back(ch);
    }
}

int WritableEntry::getMaxRightWidth(const Font* font) const
{
    return font->getWidth(L"M")*(15 - static_cast<int>(m_string.size()));
}

/*** WORLDENTRY ***/

void WorldEntry::click(int button) 
{ 
    if ((button == GLFW_MOUSE_BUTTON_LEFT) || (button == GLFW_KEY_ENTER) || (button == GLFW_KEY_KP_ENTER))
    {
        m_current = m_switchTo;
        m_menu->setState(State::World);
    }
}

/*** QUITENTRY ***/

void QuitEntry::click(int button) 
{ 
    if ((button == GLFW_MOUSE_BUTTON_LEFT) || (button == GLFW_KEY_ENTER) || (button == GLFW_KEY_KP_ENTER))
    {
        m_menu->setState(State::Quit);
    }
}

void SubmenuEntry::click(int button) 
{ 
    if ((button == GLFW_MOUSE_BUTTON_LEFT) || (button == GLFW_KEY_ENTER) || (button == GLFW_KEY_KP_ENTER))
    {
        m_menu->setSubmenu(m_submenuToSwitchTo); 
    }
}
