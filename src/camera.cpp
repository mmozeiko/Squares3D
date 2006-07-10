#include "camera.h"
#include "common.h"

#include <GL/glfw.h>

Camera::Camera(const Vector& pos, float angleX, float angleY) :
    _pos(-pos), _angleX(angleX), _angleY(angleY),
    _strafeRotation(Matrix::rotateY(-M_PI/2)),
    _scaleMatrix(Matrix::scale(Vector(1.0f, 1.0f, -1.0f)))
{
}

Camera::~Camera()
{
}

void Camera::Control(float delta)
{
    int w, h, x, y;
    glfwGetWindowSize(&w, &h);
    glfwGetMousePos(&x, &y);
    int w2 = w/2, h2 = h/2;
    int dx = x-w2, dy = y-h2;
    if (w2==0 || h2==0) return;
    if (std::abs(dx)>w2 || std::abs(dy)>h2)
    {
        glfwSetMousePos(w2, h2);
        return;
    }

    if (dx != 0 || dy != 0)
    {
        Rotate(
            delta * M_PI * LOOK_SPEED * dy / h2,
            delta * M_PI * LOOK_SPEED * dx / w2);

        glfwSetMousePos(w2, h2);
    }
    float dist = 0.0f;
    float strafe = 0.0f;

    if (glfwGetKey(GLFW_KEY_UP)==GLFW_PRESS) dist += 1.0f;
    if (glfwGetKey(GLFW_KEY_DOWN)==GLFW_PRESS) dist -= 1.0f;
    if (glfwGetKey(GLFW_KEY_RIGHT)==GLFW_PRESS) strafe += 1.0f;
    if (glfwGetKey(GLFW_KEY_LEFT)==GLFW_PRESS) strafe -= 1.0f;

    if (dist!=0 || strafe!=0)
    {
        Move(delta * MOVE_SPEED * dist, delta * MOVE_SPEED * strafe);
    }
}

void Camera::Move(float distance, float strafe)
{
    Matrix moveMatrix = Matrix::rotateY(-_angleY) * Matrix::rotateX(-_angleX);
    Matrix strafeMatrix = moveMatrix * _strafeRotation;

    Vector deltaPos = distance * moveMatrix.row(2) + strafe * strafeMatrix.row(2);
    _pos += deltaPos;
}

void Camera::Rotate(float ax, float ay)
{
    _angleX += ax;
    _angleY += ay;
}

void Camera::Prepare()
{
    _matrix = Matrix::rotateX(_angleX) * Matrix::rotateY(_angleY) * 
              Matrix::translate( _pos ) * _scaleMatrix;
}

void Camera::Render() const
{
    glLoadMatrixf(_matrix.m);
}
