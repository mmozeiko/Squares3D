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

StartGameEntry::StartGameEntry(const Vector& position, const wstring& stringIn, Menu* menu, const Font* font) : 
    Entry(position, stringIn, font),
    m_menu(menu)
{
}

wstring StartGameEntry::getString()
{
    return m_string;
}    

void StartGameEntry::click()
{
    m_menu->m_goToGame = true;
}

void Submenu::control()
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
            if (Input::instance->popButton() == GLFW_MOUSE_BUTTON_1)
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

Submenu::Submenu() : 
    m_activeEntry(NULL)
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

Menu::Menu():
    m_font(Font::get("Arial_32pt_bold")),
    m_goToGame(false),
    m_quitGame(false)
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

    Vector submenuPosition = Vector(static_cast<float>(resX) / 2,
                                    static_cast<float>(resY) / 2, 
                                    0);

    Submenu* submenu = new Submenu();

    for (int i = 0; i < 4; i++)
    {
        if (i == 0)
        {
            submenu->addEntry(new StartGameEntry(submenuPosition, entryStrings[i], this, m_font));
        }
        else
        {
            submenu->addEntry(new OptionEntry(submenuPosition, entryStrings[i], value, m_font));
        }
        submenuPosition -= Vector(0, static_cast<float>(m_font->getHeight()) + 2, 0);
    }

    m_currentSubmenu = submenu;

    m_submenus.push_back(submenu);
    
}

Menu::~Menu()
{
    Input::instance->endButtonBuffer();

    for each_const(Submenus, m_submenus, iter)
    {
        delete *iter;
    }

    delete m_backGround;
}

State::Type Menu::progress() const
{
    if (m_goToGame)
    {
        return State::World;
    }
    if (m_quitGame)
    {
        return State::Quit;
    }

    return State::Current;
}

void Menu::control()
{
    for each_const(Submenus, m_submenus, iter)
    {
        (*iter)->control();
    }
    if (Input::instance->key(GLFW_KEY_ENTER))
    {
        m_goToGame = true;
    }
    if (Input::instance->key(GLFW_KEY_ESC))
    {
        m_quitGame = true;
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

    m_currentSubmenu->render();

    m_font->end();
}
