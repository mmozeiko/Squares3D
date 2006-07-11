#include "camera.h"
#include "common.h"
#include "input.h"

#include <GL/glfw.h>

Camera::Camera(const Vector& pos, float angleX, float angleY) :
    m_pos(-pos), m_angleX(angleX), m_angleY(angleY),
    m_strafeRotation(Matrix::rotateY(-M_PI/2)),
    m_scaleMatrix(Matrix::scale(Vector(1.0f, 1.0f, -1.0f)))
{
    int w, h; 
    glfwGetWindowSize(&w, &h);
    m_lastMouse = make_pair(w/2, h/2);
}

Camera::~Camera()
{
}

void Camera::control(const Input* input, float delta)
{
    const Mouse& mouse = input->mouse();
    int dx = mouse.x - m_lastMouse.first;
    int dy = mouse.y - m_lastMouse.second;

    m_lastMouse = make_pair(mouse.x, mouse.y);

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

    if (input->key(GLFW_KEY_UP)) dist += 1.0f;
    if (input->key(GLFW_KEY_DOWN)) dist -= 1.0f;
    if (input->key(GLFW_KEY_RIGHT)) strafe += 1.0f;
    if (input->key(GLFW_KEY_LEFT)) strafe -= 1.0f;

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
