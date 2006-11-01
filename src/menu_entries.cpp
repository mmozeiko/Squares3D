#include <GL/glfw.h>

#include "menu_entries.h"
#include "menu_submenu.h"
#include "menu.h"

#include "language.h"
#include "input.h"
#include "video.h"
#include "colors.h"
#include "geometry.h"
#include "sound.h"
#include "network.h"
#include "profile.h"
#include "game.h"
#include "config.h"

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

Entry::Entry(Menu* menu, const wstring& stringIn, Font::AlignType align, const Font* font) : 
    m_string(stringIn), m_enabled(true), 
    m_menu(menu), m_forBounds(false), m_font(font), m_align(align)
{
    if (m_font == NULL)
    {
        m_font = menu->m_font;
    }
}

bool Entry::isMouseOver(const Vector& mousePos) const
{
    return isPointInRectangle(mousePos, m_lowerLeft, m_upperRight);
}

void Entry::calculateBounds(const Vector& position)
{
    m_forBounds = true;
    m_lowerLeft = Vector(position.x - m_font->getWidth(getString())/2.0f, 0, position.y);
    m_upperRight = Vector(position.x + m_font->getWidth(getString())/2.0f, 0, position.y + m_font->getHeight(getString()));
    m_forBounds = false;
}

void Entry::setXBound(float minX, float maxX)
{
    m_lowerLeft.x = minX;
    m_upperRight.x = maxX;
}

void Entry::render() const
{
    m_font->render(getString(), m_align);
}

int Entry::getMaxLeftWidth() const
{
    return m_font->getWidth(m_string)/2;
}

int Entry::getMaxRightWidth() const
{
    return m_font->getWidth(m_string)/2;
}

/*** COLORENTRY ***/

void ColorEntry::render() const
{
    wstring stringToRender = m_string + L":  ";
    Vector color = colors.find(m_value.getCurrent())->second;
    m_font->render(stringToRender, Font::Align_Right);

    glColor3f(color.x, color.y, color.z);
    
    const float h = static_cast<float>(m_font->getHeight());
    const float d = static_cast<float>(m_font->getWidth(L" ")); 
    
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
        m_menu->m_sound->play(m_menu->m_soundChange);
    }
    m_binding = colors.find(m_value.getCurrent())->second;
}

int ColorEntry::getMaxRightWidth() const
{
    return 100 + m_font->getWidth(L" ");
}

/*** WRITEABLE ENTRY ***/

void WritableEntry::render() const
{
    m_font->render(m_string + L":  ", Font::Align_Right);

    wstring stringToRender = wcast<wstring>(m_binding);
    if (m_ownerSubmenu->m_entries[m_ownerSubmenu->m_activeEntry] == this)
    {
        if (fmodf(m_timer.read(), 1.0f) > 0.5f)
        {
            stringToRender.push_back('_');
        }
    }
    m_font->render(stringToRender, Font::Align_Left);
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
    int x = m_maxBindingSize;
    if (x == -1)
    {
        x = 15 - static_cast<int>(m_string.size());
    }

    if (ch<=127 && m_menu->m_font->hasChar(ch) && (static_cast<int>(m_binding.size()) < x))
    {
        m_binding.push_back(ch);
    }
}

int WritableEntry::getMaxRightWidth() const
{
    int x = m_maxBindingSize;
    if (x == -1)
    {
        x = 15 - static_cast<int>(m_string.size());
    }

    return m_font->getWidth(L"M") * x;
}

/*** WORLDENTRY ***/

void WorldEntry::click(int button) 
{ 
    if ((button == GLFW_MOUSE_BUTTON_LEFT) || (button == GLFW_KEY_ENTER) || (button == GLFW_KEY_KP_ENTER))
    {
        Network::instance->setPlayerProfile(Game::instance->m_userProfile);
        Network::instance->setCpuProfiles(Game::instance->m_cpuProfiles, m_switchTo);

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
        if (m_back)
        {
            m_menu->m_sound2->play(m_menu->m_soundBackClick);
        }
        else
        {
            m_menu->m_sound2->play(m_menu->m_soundClick);
        }
    }
}

wstring LabelEntry::getString() const
{
    glColor3fv(Vector::One.v);
    return m_string;
}

wstring NetPlayerEntry::getString() const
{
    if (m_forBounds)
    {
        return L"MMMMMMMMMMMMMMM";
    }

    const vector<Profile*>& profiles = Network::instance->getCurrentProfiles();

    wstring type;
    if (m_idx == Network::instance->getLocalIdx())
    {
        type = Language::instance->get(TEXT_LOCAL_PLAYER);
    }
    else if (Network::instance->isLocal(m_idx))
    {
        type = Language::instance->get(TEXT_CPU_PLAYER);
    }
    else
    {
        type = Language::instance->get(TEXT_REMOTE_PLAYER);
    }

    return wcast<wstring>(profiles[m_idx]->m_name) + type;
}

void NetPlayerEntry::render() const
{
    glPushMatrix();
    glTranslatef(static_cast<float>(- this->getMaxLeftWidth()), 0.0f, 0.0f);
    m_font->render(getString(), Font::Align_Left);
    glPopMatrix();
}

void NetPlayerEntry::click(int button)
{
    if ( ! (button == GLFW_KEY_ENTER || button == GLFW_KEY_KP_ENTER || button == GLFW_MOUSE_BUTTON_1 || button == GLFW_MOUSE_BUTTON_2  || button == GLFW_KEY_RIGHT  || button == GLFW_KEY_LEFT) )
    {
        return;
    }

    if (m_idx != Network::instance->getLocalIdx() && !Network::instance->isLocal(m_idx))
    {
        // kick
        Network::instance->setAiProfile(m_idx, Network::instance->getRandomAI());
        Network::instance->kickClient(m_idx);

        m_menu->m_sound2->play(m_menu->m_soundClick);
    }
    else if (m_idx != Network::instance->getLocalIdx())
    {
        // change CPU
        Network::instance->changeCpu(m_idx, button == GLFW_MOUSE_BUTTON_1 || button == GLFW_KEY_ENTER || button == GLFW_KEY_KP_ENTER || button == GLFW_KEY_RIGHT);
        Network::instance->updateAiProfile(m_idx);

        m_menu->m_sound->play(m_menu->m_soundChange);
    }
}

void NewHostEntry::click(int button) 
{ 
    if (button == GLFW_KEY_ENTER || button == GLFW_KEY_KP_ENTER || button == GLFW_MOUSE_BUTTON_1)
    {
        Network::instance->createServer();
        Network::instance->setPlayerProfile(Game::instance->m_userProfile);
        Network::instance->setCpuProfiles(Game::instance->m_cpuProfiles, -1);        
    }
    SubmenuEntry::click(button);
}

void JoinHostEntry::click(int button) 
{ 
    if (button == GLFW_KEY_ENTER || button == GLFW_KEY_KP_ENTER || button == GLFW_MOUSE_BUTTON_1)
    {
        Network::instance->createClient();
        Network::instance->createRemoteProfiles();
    }
    SubmenuEntry::click(button);
}

void CloseHostEntry::click(int button) 
{
    if (button == GLFW_KEY_ENTER || button == GLFW_KEY_KP_ENTER || button == GLFW_MOUSE_BUTTON_1)
    {
        Network::instance->close();
    }
    SubmenuEntry::click(button);
}


wstring NetRemotePlayerEntry::getString() const
{
    if (m_forBounds)
    {
        return L"MMMMMMMMMMMMMMM";
    }

    const vector<Profile*>& profiles = Network::instance->getCurrentProfiles();

    wstring type;
    if (m_idx == Network::instance->getLocalIdx())
    {
        type = Language::instance->get(TEXT_LOCAL_PLAYER);
    }
    else
    {
        type = Language::instance->get(TEXT_REMOTE_PLAYER);
    }

    return wcast<wstring>(profiles[m_idx]->m_name) + type;
}

void NetRemotePlayerEntry::click(int button)
{
    return;
}

void NetRemotePlayerEntry::render() const
{
    glPushMatrix();
    glTranslatef(static_cast<float>(- this->getMaxLeftWidth()), 0.0f, 0.0f);
    m_font->render(getString(), Font::Align_Left);
    glPopMatrix();
}

ConnectEntry::ConnectEntry(Menu* menu, const wstring& label, const string& submenuToSwitchTo, Submenu* owner) :
    SubmenuEntry(menu, label, false, submenuToSwitchTo),
    m_owner(owner)
{
}

void ConnectEntry::click(int button)
{
    if (button == GLFW_KEY_ENTER || button == GLFW_KEY_KP_ENTER || button == GLFW_MOUSE_BUTTON_1)
    {
        m_menu->m_sound2->play(m_menu->m_soundClick);
        if (Network::instance->connect(Config::instance->m_misc.last_address))
        {
            for each_(Entries, m_owner->m_entries, iter)
            {
                Entry* entry = *iter;
                if (entry != m_owner->m_entries.back())
                {
                    entry->disable();
                }

            }
            m_string = Language::instance->get(TEXT_CONNECTING);
            m_owner->activateNextEntry(true);
        }
    }
}

/*** MULTIWORLDENTRY ***/

void MultiWorldEntry::click(int button) 
{ 
    if ((button == GLFW_MOUSE_BUTTON_LEFT) || (button == GLFW_KEY_ENTER) || (button == GLFW_KEY_KP_ENTER))
    {
        Network::instance->startGame();
        m_menu->setState(State::World);
    }
}
