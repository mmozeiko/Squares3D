#include <GL/glfw.h>

#include "camera.h"
#include "input.h"
#include "world.h"
#include "level.h"
#include "body.h"

static const float LOOK_SPEED = 0.2f;
static const float MOVE_SPEED = 5.0f;

Camera::Camera(const Vector& pos, float angleX, float angleY) :
    m_pos(-pos), 
    m_angleX(angleX * DEG_IN_RAD), 
    m_angleY(angleY * DEG_IN_RAD),
    m_strafeRotation(Matrix::rotateY(-M_PI/2)),
    m_scaleMatrix(Matrix::scale(Vector(1.0f, 1.0f, -1.0f))),
    m_lastDown(false)
{
    int w, h; 
    glfwGetWindowSize(&w, &h);
    m_lastMouse = Vector(static_cast<float>(w/2), static_cast<float>(h/2), static_cast<float>(glfwGetMouseWheel()));
}

Camera::~Camera()
{
}

bool userControlled = false;

void Camera::control()
{
    const Mouse& mouse = Input::instance->mouse();
    m_targetRotation = Vector::Zero;
    //m_targetDirection = Vector::Zero;

    if (m_lastDown == false && (mouse.b & 2) == 2)
    {
        m_lastMouse = Vector(static_cast<float>(mouse.x/2), static_cast<float>(mouse.y/2), static_cast<float>(mouse.z/2));
        m_lastDown = true;
    }
    if (m_lastDown == true && (mouse.b & 2) == 0)
    {
        m_lastDown = false;
    }

    if ((mouse.b & 2) == 2)
    {
        Vector newMouse(static_cast<float>(mouse.x/2), static_cast<float>(mouse.y/2), static_cast<float>(mouse.z/2));

        Vector delta  = newMouse - m_lastMouse;

        m_targetRotation.y = delta.x;
        m_targetRotation.x = delta.y;
        //m_targetDirection.z = delta.z;

        m_lastMouse = newMouse;

    }
}

void Camera::update(float delta)
{
    m_targetRotation *= delta;
    m_targetDirection *= delta;

    m_angleY += LOOK_SPEED * m_targetRotation.y;
    m_angleX += LOOK_SPEED * m_targetRotation.x;
    
    if (m_angleX < 10.0f * DEG_IN_RAD)
    {
        m_angleX = 10.0f * DEG_IN_RAD;
    }
    else if (m_angleX > 60.0f * DEG_IN_RAD)
    {
        m_angleX = 60.0f * DEG_IN_RAD;
    }

/*
    Matrix moveMatrix = Matrix::rotateX(m_angleX) * Matrix::rotateY(m_angleY);
    //Matrix strafeMatrix = moveMatrix * m_strafeRotation;

    Vector deltaPos = m_targetDirection.z * moveMatrix.row(2); // + m_targetDirection.x * strafeMatrix.row(2);

    m_pos += MOVE_SPEED * deltaPos;

    float m = m_pos.magnitude();
    if (m < 3.0f)
    {
        m_pos.norm();
        m_pos *= 3.0f;
    }
    else if (m > 10.0f)
    {
        m_pos.norm();
        m_pos *= 10.0f;
    }
*/
}

void Camera::prepare()
{
    m_matrix = Matrix::translate(m_pos)  * 
               Matrix::rotateX(m_angleX) *
               Matrix::rotateY(m_angleY) *
               m_scaleMatrix;
}

void Camera::render() const
{
    glLoadMatrixf(m_matrix.m);
}

float Camera::angleY() const
{
    return m_angleY;
}
