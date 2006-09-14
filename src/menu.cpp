#include <GL/glfw.h>

#include "menu.h"
#include "game.h"
#include "world.h"
#include "font.h"
#include "video.h"
#include "texture.h"

Entry::Entry(Vector& position) :
    m_position(position)
{
    
}

void Entry::render(const Font* font) const
{
    glPushMatrix();
    glTranslatef(m_position.x, m_position.y, m_position.z);
    glColor3fv(Vector::One.v);
    font->render(m_string +  L" " + m_value.m_string, Font::Align_Center);
    glPopMatrix();
}

void Entry::control()
{
}

SubMenu::SubMenu(Vector& position) : 
    m_position(position)
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

void SubMenu::render(const Font* font) const
{
    glPushMatrix();
    glTranslatef(m_position.x, m_position.y, m_position.z);
    for each_const(Entries, m_entries, iter)
    {
        (*iter)->render(font);
    }
    glPopMatrix();
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

    m_backGroundTexture = Video::instance->loadTexture("boob");

    loadMenuTree();
}

void Menu::loadMenuTree()
{
    Value value;
    value.m_string = L"valueName";

    Entry* entry = new Entry(Vector(Vector::Zero));
    entry->m_string = L"entryName";
    entry->m_value = value;

    SubMenu* subMenu = new SubMenu(Vector(400, 400, 0));

    subMenu->addEntry(entry);

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

    m_currentSubMenu->render(m_font);
    //for each_const(SubMenus, m_subMenus, iter)
    //{
    //    (*iter)->render(m_font);
    //}
    m_font->end();
}
