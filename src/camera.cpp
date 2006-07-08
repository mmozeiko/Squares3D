#include "camera.h"

#include <GL/glfw.h>

Camera::Camera(const Vector& pos, float angleX, float angleY) :
    _pos(-pos), _angleX(angleX), _angleY(angleY),
    _strafeRotation(Matrix::rotateY(-M_PI/2)),
    _scaleMatrix(Matrix::scale(Vector(1.0f, 1.0f, -1.0f)))
{
}

void Camera::Control(float delta)
{
    int w, h;
    glfwGetWindowSize(&w, &h);
    int x, y;
    glfwGetMousePos(&x, &y);
    int w2 = w/2, h2 = h/2;

    if (x!=w2 || y !=h2)
    {
        Rotate(
            delta * M_PI * LOOK_SPEED * (y-h2) / h2,
            delta * M_PI * LOOK_SPEED * (x-w2) / w2);

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
