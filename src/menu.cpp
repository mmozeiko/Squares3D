#include <GL/glfw.h>

#include "menu.h"
#include "game.h"
#include "world.h"
#include "font.h"
#include "video.h"
#include "texture.h"


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

}

Menu::~Menu()
{
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
    m_font->end();
}
