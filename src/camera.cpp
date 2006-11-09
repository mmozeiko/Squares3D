#include "camera.h"
#include "input.h"
#include "world.h"
#include "level.h"
#include "body.h"

static const float LOOK_SPEED = 0.2f;
static const float MOVE_SPEED = 10.0f;

Camera::Camera(const Vector& pos, float angleX, float angleY) :
    m_targetRotation(),
    m_targetDirection(),
    m_pos(-pos), 
    m_angleX(angleX * DEG_IN_RAD), 
    m_angleY(angleY * DEG_IN_RAD),
    m_strafeRotation(Matrix::rotateY(-M_PI/2)),
    m_scaleMatrix(Matrix::scale(Vector(1.0f, 1.0f, -1.0f))),
    m_matrix(),
    m_lastDown(false),
    m_uuberCamera(false),
    m_lastUuberKey(false),
    m_defPos(-pos),
    m_defAngleX(angleX * DEG_IN_RAD),
    m_defAngleY(angleY * DEG_IN_RAD)
{
}

Camera::~Camera()
{
}

bool userControlled = false;

void Camera::control()
{
    const Mouse& mouse = Input::instance->mouse();
    m_targetRotation = Vector::Zero;
    m_targetDirection = Vector::Zero;

    if (m_lastDown == false && (mouse.b & 2) == 2)
    {
        m_lastDown = true;
    }
    if (m_lastDown == true && (mouse.b & 2) == 0)
    {
        m_lastDown = false;
    }

    if ((mouse.b & 2) == 2)
    {
        Vector newMouse(static_cast<float>(mouse.x), static_cast<float>(mouse.y), static_cast<float>(mouse.z));
        Vector delta  = newMouse;

        m_targetRotation.y = delta.x;
        m_targetRotation.x = delta.y;

        if (m_uuberCamera)
        {
            if (Input::instance->key(GLFW_KEY_UP))       m_targetDirection.z = +1.0f;
            if (Input::instance->key(GLFW_KEY_DOWN))     m_targetDirection.z = -1.0f;
            if (Input::instance->key(GLFW_KEY_RIGHT))    m_targetDirection.x = +1.0f;
            if (Input::instance->key(GLFW_KEY_LEFT))     m_targetDirection.x = -1.0f;

            m_targetDirection.norm();
        }
    }

    if (m_uuberCamera == false)
    {
        m_targetDirection = Vector::Zero;
        m_targetDirection.z = static_cast<float>(mouse.z);
    }

    if (Input::instance->key(GLFW_KEY_TAB) && m_lastUuberKey==false)
    {
        m_uuberCamera = !m_uuberCamera;

        if (m_uuberCamera)
        {
            // .. recalc ??
        }
        else
        {
            m_pos = m_defPos;
            m_angleX = m_defAngleX;
            m_angleY = m_defAngleY;
        }
        
        m_targetRotation = Vector::Zero;
        m_targetDirection = Vector::Zero;

        m_lastUuberKey = true;
    }

    if (Input::instance->key(GLFW_KEY_TAB)==false && m_lastUuberKey==true)
    {
        m_lastUuberKey = false;
    }

}

void Camera::update(float delta)
{
    if (m_uuberCamera)
    {
        m_targetRotation *= delta;
        m_targetDirection *= delta;

        m_angleY += LOOK_SPEED * m_targetRotation.y;
        m_angleX += LOOK_SPEED * m_targetRotation.x;

        const Matrix moveMatrix = Matrix::rotateY(-m_angleY) * Matrix::rotateX(-m_angleX);
        const Matrix strafeMatrix = moveMatrix * m_strafeRotation;

        const Vector deltaPos = m_targetDirection.z * moveMatrix.row(2) + m_targetDirection.x * strafeMatrix.row(2);
        m_pos += MOVE_SPEED * deltaPos;
    }
    else
    {
        m_targetRotation *= delta;
        m_targetDirection *= delta;

        float t = 50.0f * LOOK_SPEED * m_targetDirection.z;
        if (t > 0)
        {
            m_pos *= 1.0f - t;
        }
        else if (t < 0)
        {
            m_pos *= 1.0f - t;

        }
        if (m_pos.z > 0.0f)
        {
            m_pos.z = -m_pos.z;
        }
        if (m_pos.magnitude2() < 4.0f*4.0f)
        {
            m_pos.norm();
            m_pos *= 4.0f;
        }
        else if (m_pos.magnitude2() > 17.0f*17.0f)
        {
            m_pos.norm();
            m_pos *= 17.0f;
        }

        m_angleY += LOOK_SPEED * m_targetRotation.y;
        m_angleX += LOOK_SPEED * m_targetRotation.x;
        
        if (m_angleX < 9.0f * DEG_IN_RAD)
        {
            m_angleX = 9.0f * DEG_IN_RAD;
        }
        else if (m_angleX > 60.0f * DEG_IN_RAD)
        {
            m_angleX = 60.0f * DEG_IN_RAD;
        }
    }
}

void Camera::prepare()
{
    if (m_uuberCamera)
    {
        m_matrix = Matrix::rotateX(m_angleX) * 
                   Matrix::rotateY(m_angleY) *
                   Matrix::translate(m_pos)  *
                   m_scaleMatrix;
    }
    else
    {
        m_matrix = Matrix::translate(m_pos)  * 
                   Matrix::rotateX(m_angleX) *
                   Matrix::rotateY(m_angleY) *
                   m_scaleMatrix;
    }
}

void Camera::render() const
{
    glLoadMatrixf(m_matrix.m);
}

float Camera::angleY() const
{
    return m_angleY;
}
