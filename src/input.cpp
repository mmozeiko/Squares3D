#include "input.h"

Input* System<Input>::instance = NULL;

Input::Input() : m_mouse(), m_keyBuffer()
{
    clog << "Initializing input... " << endl;

    glfwGetWindowSize(&m_mouse.x, &m_mouse.y);
    m_mouse.x /= 2;
    m_mouse.y /= 2;
    m_mouse.z = 0;
    m_mouse.b = 0;

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
    m_mouse.b = 0;
    for (int b=GLFW_MOUSE_BUTTON_1; b<=GLFW_MOUSE_BUTTON_LAST; b++)
    {
        if (glfwGetMouseButton(b) == GLFW_PRESS)
        {
            m_mouse.b |= 1 << (b-GLFW_MOUSE_BUTTON_1);
        }
    }
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

void Input::startKeyBuffer()
{
    m_keyBuffer.clear();
    glfwSetKeyCallback(keyFunc);
}

const IntVector& Input::getKeyBuffer() const
{
    return m_keyBuffer;
}

void Input::endKeyBuffer()
{
    glfwSetCharCallback(NULL);
}

void Input::addKey(int c)
{
    m_keyBuffer.push_back(c);
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
