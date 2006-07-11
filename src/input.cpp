#include "input.h"

#include <manymouse.h>


Input::Input() :
    m_mouseCount(0), m_manymouses(false), m_mouses(), m_keyBuffer()
{
    clog << "Initializing input... ";

    int tmp = ManyMouse_Init();
    if (tmp > 0)
    {
        m_manymouses = true;
        m_mouseCount = tmp;
    }
    else
    {
        m_mouseCount = 1;
    }

    Mouse defaultMouse;
    glfwGetWindowSize(&defaultMouse.x, &defaultMouse.y);
    defaultMouse.x /= 2;
    defaultMouse.y /= 2;
    defaultMouse.z = 0;
    defaultMouse.b = 0;

    glfwSetMousePos(defaultMouse.x, defaultMouse.y);
    glfwSetMouseWheel(0);

    m_mouses.resize(m_mouseCount, defaultMouse);

    clog << m_mouseCount << ' ' << (m_mouseCount>1 ? "mices" : "mouse" ) << " found." << endl;
}

Input::~Input()
{
    clog << "Closing input." << endl;

    ManyMouse_Quit();
}

void Input::process()
{
    if (m_manymouses)
    {
        ManyMouseEvent event;
        while (ManyMouse_PollEvent(&event))
        {
            switch (event.type)
            {
                case MANYMOUSE_EVENT_ABSMOTION:
                    if (event.item == 0)
                    {
                        m_mouses[event.device].x = event.value;
                    }
                    else
                    {
                        m_mouses[event.device].y = event.value;
                    }
                    break;
                case MANYMOUSE_EVENT_RELMOTION:
                    if (event.item == 0)
                    {
                        m_mouses[event.device].x += event.value;
                    }
                    else
                    {
                        m_mouses[event.device].y += event.value;
                    }
                    break;
                case MANYMOUSE_EVENT_BUTTON:
                    if (event.value != 0)
                    {
                        m_mouses[event.device].b |= (1 << event.item);
                    }
                    else
                    {
                        m_mouses[event.device].b &= ~(1 << event.item);
                    }
                    break;
                case MANYMOUSE_EVENT_SCROLL:
                    if (event.item == 0)
                    {
                        m_mouses[event.device].z += event.value;
                    }
                    break;
            }
        }
    }
    else
    {
        glfwGetMousePos(&m_mouses[0].x, &m_mouses[0].y);
        m_mouses[0].z = glfwGetMouseWheel();
        m_mouses[0].b = 0;
        for (int b=GLFW_MOUSE_BUTTON_1; b<=GLFW_MOUSE_BUTTON_LAST; b++)
        {
            if (glfwGetMouseButton(b) == GLFW_PRESS)
            {
                m_mouses[0].b |= 1 << (b-GLFW_MOUSE_BUTTON_1);
            }
        }
    }
}

bool Input::key(int key) const
{
    return glfwGetKey(key) == GLFW_PRESS;
}

const Mouse& Input::mouse(int id) const
{
    if (id >= m_mouseCount)
    {
        string sid = cast<int, string>(id);
        throw Exception("Invalid mouse id - " + id);
    }
    return m_mouses[id];
}


void Input::startKeyBuffer()
{
    m_keyBuffer = "";
}

string Input::getKeyBuffer() const
{
    return m_keyBuffer;
}

void Input::endKeyBuffer()
{
}
