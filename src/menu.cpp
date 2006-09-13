#include <GL/glfw.h>

#include "menu.h"
#include "game.h"
#include "world.h"


Menu::Menu():
    m_goToGame(false)
{
}

Menu::~Menu()
{
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
}
