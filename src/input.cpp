#include "input.h"

Input* System<Input>::instance = NULL;

Input::Input() : m_mouse(), m_keyBuffer(), m_buttonBuffer()
{
    clog << "Initializing input... " << endl;

    glfwGetWindowSize(&m_mouse.x, &m_mouse.y);
    m_mouse.x /= 2;
    m_mouse.y /= 2;
    m_mouse.z = 0;

    glfwSetMousePos(m_mouse.x, m_mouse.y);
    glfwSetMouseWheel(0);
}

Input::~Input()
{
    clog << "Closing input." << endl;
}

void Input::update()
{
    glfwGetMousePos(&m_mouse.x, &m_mouse.y);
    m_mouse.z = glfwGetMouseWheel();
}

bool Input::key(int key) const
{
    return glfwGetKey(key) == GLFW_PRESS;
}

const Mouse& Input::mouse() const
{
    return m_mouse;
}

void GLFWCALL keyFunc(int key, int action)
{
    if (action == GLFW_PRESS)
    {
        Input::instance->addKey(key);
    }
}

void GLFWCALL buttonFunc(int button, int action)
{
    if (action == GLFW_PRESS)
    {
        Input::instance->addButton(button);
    }
}

void Input::startKeyBuffer()
{
    m_keyBuffer.clear();
    glfwSetKeyCallback(keyFunc);
}

void Input::endKeyBuffer()
{
    glfwSetKeyCallback(NULL);
}

void Input::addKey(int k)
{
    m_keyBuffer.push_back(k);
}

int Input::popKey()
{
    if (m_keyBuffer.size() == 0)
    {
        return -1;
    }
    int key = m_keyBuffer.front();
    m_keyBuffer.erase(m_keyBuffer.begin());
    return key;
}

void Input::startButtonBuffer()
{
    m_buttonBuffer.clear();
    glfwSetMouseButtonCallback(buttonFunc);
}

void Input::endButtonBuffer()
{
    glfwSetMouseButtonCallback(NULL);
}

void Input::addButton(int b)
{
    m_buttonBuffer.push_back(b);
}

int Input::popButton()
{
    if (m_buttonBuffer.size() == 0)
    {
        return -1;
    }
    int b = m_buttonBuffer.front();
    m_buttonBuffer.erase(m_buttonBuffer.begin());
    return b;
}
