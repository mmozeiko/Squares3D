#include "input.h"

template <class Input> Input* System<Input>::instance = NULL;

Input::Input() : m_mouseVisible(true), m_mouse(), m_keyBuffer(), m_charBuffer(), m_buttonBuffer()
{
    clog << "Initializing input... " << endl;
}

Input::~Input()
{
    clog << "Closing input." << endl;
}

void Input::init()
{
    glfwGetWindowSize(&m_mouse.x, &m_mouse.y);
    m_mouse.x /= 2;
    m_mouse.y /= 2;
    m_mouse.z = 0;

    m_mouseMiddleX = m_mouse.x;
    m_mouseMiddleY = m_mouse.y;

    glfwSetMousePos(m_mouse.x, m_mouse.y);
    glfwSetMouseWheel(0);
}

void Input::mouseVisible(bool visible)
{
    if (visible)
    {
        glfwEnable(GLFW_MOUSE_CURSOR);
        //glfwSetMousePos(m_mouseMiddleX, m_mouseMiddleY);
    }
    else
    {
        glfwDisable(GLFW_MOUSE_CURSOR);
        //glfwSetMousePos(0, 0); //m_mouseMiddleX, m_mouseMiddleY);
    }
    m_mouseVisible = visible;
}

void Input::update()
{
    glfwGetMousePos(&m_mouse.x, &m_mouse.y);
    m_mouse.z = glfwGetMouseWheel();
    if (m_mouseVisible == false)
    {
	    //int x = m_lastX;
		//int y = m_lastY;
        //clog << "MOUSE POS = " << m_mouse.x << ' ' << m_mouse.y << endl;
		//m_mouse.x -= m_mouseMiddleX;
        //m_mouse.y -= m_mouseMiddleY;
        //glfwSetMousePos(0, 0); //m_mouseMiddleX, m_mouseMiddleY);
    }
    
    m_mouse.b = 0;
    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_1))
    {
        m_mouse.b |= 1;
    }
    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_2))
    {
        m_mouse.b |= 2;
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

static void GLFWCALL keyFunc(int key, int action)
{
    if (action == GLFW_PRESS)
    {
        Input::instance->addKey(key);
    }
}

static void GLFWCALL charFunc(int ch, int action)
{
    if (action == GLFW_PRESS)
    {
        Input::instance->addChar(ch);
    }
}

static void GLFWCALL buttonFunc(int button, int action)
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

void Input::startCharBuffer()
{
    m_charBuffer.clear();
    glfwSetCharCallback(charFunc);
}

void Input::endCharBuffer()
{
    glfwSetCharCallback(NULL);
}

void Input::addChar(int k)
{
    m_charBuffer.push_back(k);
}

int Input::popKey()
{
    if (m_keyBuffer.size() == 0)
    {
        return -1;
    }
    int key = m_keyBuffer.front();
    m_keyBuffer.pop_front();
    return key;
}

int Input::popChar()
{
    if (m_charBuffer.size() == 0)
    {
        return -1;
    }
    int ch = m_charBuffer.front();
    m_charBuffer.pop_front();
    return ch;
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
    m_buttonBuffer.pop_front();
    return b;
}
