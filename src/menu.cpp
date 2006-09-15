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

void Value::addAnother(const wstring& string)
{
    m_values.push_back(string);
}

Value::Value(const wstring& string) :
    m_current(0)
{
    m_values.push_back(string);
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

OptionEntry::OptionEntry(const Vector& position, const wstring& stringIn, const Value& value, const Font* font) : 
    Entry(position, stringIn, font),
    m_value(value)
{
}

wstring OptionEntry::getString()
{
    return m_string +  L": " + m_value.getCurrent();
}

void OptionEntry::click()
{
    m_value.activateNext();
}

MenuEntry::MenuEntry(const Vector& position, const wstring& stringIn, const Font* font) : 
    Entry(position, stringIn, font)
{
}

wstring MenuEntry::getString()
{
    return m_string;
}    

void MenuEntry::click()
{
    //m_value.activateNext();
}

void SubMenu::control()
{
    bool onAnyEntry = false;
    for each_const(Entries, m_entries, iter)
    {
        const Mouse& mouse = Input::instance->mouse();
        int videoHeight = Video::instance->getResolution().second;
        Vector mousePos = Vector(static_cast<float>(mouse.x), 0, static_cast<float>(videoHeight - mouse.y));

        if (isPointInRectangle(mousePos, (*iter)->m_lowerLeft, (*iter)->m_upperRight))
        {
            onAnyEntry = true;
            m_activeEntry = (*iter);
            if (mouse.b & 1)
            {
                m_activeEntry->click();
            }
        }
    }
    if (!onAnyEntry)
    {
        m_activeEntry = NULL;
    }
}

SubMenu::SubMenu() : 
    m_activeEntry(NULL)
{
}

SubMenu::~SubMenu()
{
    for each_const(Entries, m_entries, iter)
    {
        delete *iter;
    }
}

void SubMenu::addEntry(Entry* entry)
{
    m_entries.push_back(entry);
}

void SubMenu::render() const
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

Menu::Menu():
    m_font(Font::get("Arial_32pt_bold")),
    m_goToGame(false)
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
}

void Menu::loadMenu()
{
    Language* language = Language::instance;

    Value value(language->get(TEXT_TRUE));
    value.addAnother(language->get(TEXT_FALSE));

    vector<wstring> entryStrings;
    entryStrings.push_back(language->get(TEXT_START_GAME));
    entryStrings.push_back(language->get(TEXT_OPTIONS));
    entryStrings.push_back(language->get(TEXT_CREDITS));
    entryStrings.push_back(language->get(TEXT_QUIT_GAME));

    int resX = Video::instance->getResolution().first;
    int resY = Video::instance->getResolution().second;

    Vector subMenuPosition = Vector(static_cast<float>(resX) / 2,
                                    static_cast<float>(resY) / 2, 
                                    0);

    SubMenu* subMenu = new SubMenu();

    for (int i = 0; i < 4; i++)
    {
        subMenu->addEntry(new OptionEntry(subMenuPosition, entryStrings[i], value, m_font));
        subMenuPosition -= Vector(0, static_cast<float>(m_font->getHeight()) + 2, 0);
    }

    m_currentSubMenu = subMenu;

    m_subMenus.push_back(subMenu);
    
}

Menu::~Menu()
{
    for each_const(SubMenus, m_subMenus, iter)
    {
        delete *iter;
    }

    delete m_backGround;
}

State::Type Menu::progress() const
{
    State::Type stateType = State::Current;
    if (m_goToGame)
    {
        stateType = State::World;
    }
    return stateType;
}

void Menu::control()
{
    for each_const(SubMenus, m_subMenus, iter)
    {
        (*iter)->control();
    }
    if (glfwGetKey(GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        m_goToGame = true;
    }
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

    m_currentSubMenu->render();

    m_font->end();
}
