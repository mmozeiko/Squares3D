#include <GL/glfw.h>

#include "menu.h"
#include "game.h"
#include "world.h"
#include "font.h"
#include "video.h"
#include "texture.h"
#include "geometry.h"
#include "input.h"
#include "language.h"
#include "config.h"

void Value::addAnother(const wstring& string)
{
    m_values.push_back(string);
}

Value::Value(const string& id) :
    m_current(0),
    m_id(id)
{
}

wstring Value::getCurrent()
{
    wstring returnVal;
    if (!m_values.empty())
    {
        returnVal = m_values[m_current];
    }
    return returnVal;
}

void Value::activateNext()
{
    if (!m_values.empty())
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
}

Entry::Entry(const Vector& position, const wstring& stringIn, const Font* font) :
    m_font(font),
    m_position(position),
    m_string(stringIn)
{
    m_lowerLeft = Vector(position.x - m_font->getWidth(stringIn), 0, position.y);
    m_upperRight = Vector(position.x + m_font->getWidth(stringIn), 
                          0,
                          position.y + m_font->getHeight());
}


string Entry::getValueID()
{
    return "";
}

size_t Entry::getCurrentValueIdx()
{
    return -1;
}

OptionEntry::OptionEntry(const Vector& position, const wstring& stringIn, const Value& value, const Font* font) : 
    Entry(position, stringIn, font),
    m_value(value)
{
}

wstring OptionEntry::getString()
{
    return m_string +  L": " + m_value.getCurrent();
}

string OptionEntry::getValueID()
{
    return m_value.m_id;
}

size_t OptionEntry::getCurrentValueIdx()
{
    return m_value.m_current;
}

void OptionEntry::click()
{
    m_value.activateNext();
}

GameEntry::GameEntry(const Vector& position, 
                     const wstring& stringIn, 
                     Menu* menu, 
                     State::Type stateToSwitchTo, 
                     const Font* font) : 
    Entry(position, stringIn, font),
    m_menu(menu),
    m_stateToSwitchTo(stateToSwitchTo)
{
}

wstring GameEntry::getString()
{
    return m_string;
}    

void GameEntry::click()
{
    m_menu->setState(m_stateToSwitchTo);
}

SubmenuEntry::SubmenuEntry(const Vector&  position, 
                           const wstring& stringIn, 
                           Menu*          menu, 
                           const string&  submenuToSwitchTo, 
                           const   Font*  font) : 
    Entry(position, stringIn, font),
    m_menu(menu),
    m_submenuToSwitchTo(submenuToSwitchTo)
{
}

wstring SubmenuEntry::getString()
{
    return m_string;
}    

void SubmenuEntry::click()
{
    m_menu->setSubmenu(m_submenuToSwitchTo);
}

ApplyOptionsEntry::ApplyOptionsEntry(const Vector&  position, 
                                     const wstring& stringIn, 
                                     Menu*          menu, 
                                     const string&  submenuToSwitchTo, 
                                     const   Font*  font) :
    SubmenuEntry(position, stringIn, menu, submenuToSwitchTo, font)
{
}

void ApplyOptionsEntry::click()
{
    //save config settings
    for each_const(Entries, m_menu->m_currentSubmenu->m_entries, iter)
    {
        string id = (*iter)->getValueID();

        if (id == "resolution")
        {
            Config::instance->m_video.width = Video::instance->getModes()[(*iter)->getCurrentValueIdx()].first;
            Config::instance->m_video.height = Video::instance->getModes()[(*iter)->getCurrentValueIdx()].second;
        }
        else if (id == "fullscreen")
        {
            Config::instance->m_video.fullscreen = (*iter)->getCurrentValueIdx();
        }
    }
    
    m_menu->setState(State::Quit);  //setSubmenu(m_submenuToSwitchTo);
    g_needsToReload = true;
}

void Submenu::control()
{
    bool onAnyEntry = false;
    bool left_button = (Input::instance->popButton() == GLFW_MOUSE_BUTTON_LEFT);

    for each_const(Entries, m_entries, iter)
    {
        const Mouse& mouse = Input::instance->mouse();
        int videoHeight = Video::instance->getResolution().second;
        Vector mousePos = Vector(static_cast<float>(mouse.x), 0, static_cast<float>(videoHeight - mouse.y));

        if (isPointInRectangle(mousePos, (*iter)->m_lowerLeft, (*iter)->m_upperRight))
        {
            onAnyEntry = true;
            m_activeEntry = (*iter);
            if (left_button)
            {
                m_activeEntry->click();
            }
            break;
        }
    }
    if (!onAnyEntry)
    {
        m_activeEntry = NULL;
    }
}

Submenu::Submenu(Vector& lastEntryPos) : 
    m_activeEntry(NULL),
    m_lastEntryPos(lastEntryPos)
{
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
    m_lastEntryPos -= Vector(0, static_cast<float>(entry->m_font->getHeight()) + 2, 0);
}

void Submenu::render() const
{
    for each_const(Entries, m_entries, iter)
    {
        glPushMatrix();
        glTranslatef((*iter)->m_position.x, (*iter)->m_position.y, (*iter)->m_position.z);
        if (m_activeEntry == *iter)
        {
            glColor3fv(Vector::One.v);
        }
        else
        {
            glColor3fv(Vector::Zero.v);
        }
        (*iter)->m_font->render((*iter)->getString(), 
                                Font::Align_Center);
        glPopMatrix();   
    }
}

Menu::Menu() :
    m_font(Font::get("Arial_32pt_bold")),
    m_state(State::Current)
{
    float resX = static_cast<float>(Video::instance->getResolution().first);
    float resY = static_cast<float>(Video::instance->getResolution().second);

    m_backGround = new Face();
    m_backGround->vertexes.push_back(Vector::Zero);
    m_backGround->vertexes.push_back(Vector(0, resY, 0));
    m_backGround->vertexes.push_back(Vector(resX, resY, 0));
    m_backGround->vertexes.push_back(Vector(resX, 0, 0));

    m_backGround->uv.push_back(UV(0, 0));
    m_backGround->uv.push_back(UV(1, 0));
    m_backGround->uv.push_back(UV(1, 1));
    m_backGround->uv.push_back(UV(0, 1));

    m_backGroundTexture = Video::instance->loadTexture("pavement");

    loadMenu();
    Input::instance->startButtonBuffer();
    Input::instance->startKeyBuffer();
}

void Menu::loadMenu()
{
    Language* language = Language::instance;

    int resX = Video::instance->getResolution().first;
    int resY = Video::instance->getResolution().second;

    Vector submenuPosition = Vector(static_cast<float>(resX) / 2,
                                    static_cast<float>(resY) / 2, 
                                    0);

    Submenu* submenu = new Submenu(submenuPosition);

    submenu->addEntry(new GameEntry(submenu->m_lastEntryPos, language->get(TEXT_START_GAME), this, State::World, m_font));
    
    submenu->addEntry(new SubmenuEntry(submenu->m_lastEntryPos, language->get(TEXT_OPTIONS), this, "options", m_font));

    submenu->addEntry(new GameEntry(submenu->m_lastEntryPos, language->get(TEXT_QUIT_GAME), this, State::Quit, m_font));

    m_currentSubmenu = submenu;
    m_submenus["main"] = submenu;

    submenu = new Submenu(submenuPosition);
    
    IntPairVector resolutions = Video::instance->getModes();
    Value valueRes("resolution");
 
    for (size_t i = 0; i < resolutions.size(); i++)
    {
        if (Video::instance->getResolution() == resolutions[i])
        {
            valueRes.m_current = i;
        }
        valueRes.addAnother(wcast<wstring>(resolutions[i].first) + L"x" + wcast<wstring>(resolutions[i].second));
    }

    submenu->addEntry(new OptionEntry(submenu->m_lastEntryPos, language->get(TEXT_RESOLUTION), valueRes, m_font));

    Value valueFS("fullscreen");
    valueFS.addAnother(language->get(TEXT_FALSE));
    valueFS.addAnother(language->get(TEXT_TRUE));
    valueFS.m_current = Config::instance->m_video.fullscreen;

    submenu->addEntry(new OptionEntry(submenu->m_lastEntryPos, language->get(TEXT_FULLSCREEN), valueFS, m_font));

    submenu->addEntry(new ApplyOptionsEntry(submenu->m_lastEntryPos, language->get(TEXT_SAVE), this, "options", m_font));

    submenu->addEntry(new SubmenuEntry(submenu->m_lastEntryPos, language->get(TEXT_BACK), this, "main", m_font));    

    m_submenus["options"] = submenu;
}

Menu::~Menu()
{
    Input::instance->endKeyBuffer();
    Input::instance->endButtonBuffer();

    for each_const(Submenus, m_submenus, iter)
    {
        delete iter->second;
    }

    delete m_backGround;
}

State::Type Menu::progress() const
{
    return m_state;
}

void Menu::setState(State::Type state)
{
     m_state = state;
}

void Menu::setSubmenu(string& submenuToSwitchTo)
{
    m_currentSubmenu = m_submenus.find(submenuToSwitchTo)->second;
}

void Menu::control()
{
    m_currentSubmenu->control();
}

void Menu::update(float delta)
{
}

void Menu::updateStep(float delta)
{
}

void Menu::prepare()
{
}

void Menu::render() const
{
    m_font->begin();

    m_backGroundTexture->begin();
    Video::instance->renderFace(*m_backGround);
    m_backGroundTexture->end();
    
    glBindTexture(GL_TEXTURE_2D, m_font->m_texture);
    glEnable(GL_TEXTURE_2D);

    m_currentSubmenu->render();

    m_font->end();
}
