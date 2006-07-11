#include "camera.h"
#include "common.h"

#include <GL/glfw.h>
#include "manymouse.h"

Camera::Camera(const Vector& pos, float angleX, float angleY) :
    m_pos(-pos), m_angleX(angleX), m_angleY(angleY),
    m_strafeRotation(Matrix::rotateY(-M_PI/2)),
    m_scaleMatrix(Matrix::scale(Vector(1.0f, 1.0f, -1.0f)))
{
    ManyMouse_Init();
}

Camera::~Camera()
{
    ManyMouse_Quit();
}

static int posX = -1;
static int posY = -1;

void Camera::control(float delta)
{
    int dx = 0;
    int dy = 0;

    ManyMouseEvent event;
    while (ManyMouse_PollEvent(&event))
    {
        if (event.device != 0)
        {
            continue;
        }
        switch (event.type)
        {
            case MANYMOUSE_EVENT_ABSMOTION:
                if (event.item == 0)
                {
                    if (posX == -1)
                    {
                        posX = event.value;
                    }
                    if (posY != -1)
                    {
                        relX += event.value - posX;
                    }
                }
                else
                {
                    if (posY == -1)
                    {
                        posY = event.value;
                    }
                    if (posX != -1)
                    {
                        relY += event.value - posY;
                    }
                }
                break;
            case MANYMOUSE_EVENT_RELMOTION:
                if (event.item == 0)
                {
                    relX += event.value;
                }
                else
                {
                    relY += event.value;
                }
                break;
        }
    }

    int w, h;
    glfwGetWindowSize(&w, &h);
    int w2 = w/2, h2 = h/2;
    if (w2==0 || h2==0)
    {
        return;
    }

    if (glfwGetWindowParam(GLFW_ACTIVE) == GL_FALSE)
    {
        return;
    }

    if (dx != 0 || dy != 0)
    {
        rotate(
            delta * M_PI * LOOK_SPEED * dy / h2,
            delta * M_PI * LOOK_SPEED * dx / w2);
    }
    float dist = 0.0f;
    float strafe = 0.0f;

    if (glfwGetKey(GLFW_KEY_UP)==GLFW_PRESS) dist += 1.0f;
    if (glfwGetKey(GLFW_KEY_DOWN)==GLFW_PRESS) dist -= 1.0f;
    if (glfwGetKey(GLFW_KEY_RIGHT)==GLFW_PRESS) strafe += 1.0f;
    if (glfwGetKey(GLFW_KEY_LEFT)==GLFW_PRESS) strafe -= 1.0f;

    if (dist!=0 || strafe!=0)
    {
        move(delta * MOVE_SPEED * dist, delta * MOVE_SPEED * strafe);
    }
}

void Camera::move(float distance, float strafe)
{
    Matrix moveMatrix = Matrix::rotateY(-m_angleY) * Matrix::rotateX(-m_angleX);
    Matrix strafeMatrix = moveMatrix * m_strafeRotation;

    Vector deltaPos = distance * moveMatrix.row(2) + strafe * strafeMatrix.row(2);
    m_pos += deltaPos;
}

void Camera::rotate(float ax, float ay)
{
    m_angleX += ax;
    m_angleY += ay;
}

void Camera::prepare()
{
    m_matrix = Matrix::rotateX(m_angleX) * Matrix::rotateY(m_angleY) * 
               Matrix::translate(m_pos) * m_scaleMatrix;
}

void Camera::render() const
{
    glLoadMatrixf(m_matrix.m);
}
