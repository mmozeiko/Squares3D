#include "camera.h"

#include <GL/glfw.h>

Camera::Camera(const Vector& pos, float angleX, float angleY) :
    _pos(pos), _angleX(angleX), _angleY(angleY),
    _strafeRotation(Matrix::rotateY(M_PI/2))
{
}

void Camera::move(float distance, float strafe)
{
    Matrix moveMatrix = Matrix::rotateX(_angleX) * Matrix::rotateY(_angleY);
    Matrix strafeMatrix = _strafeRotation * moveMatrix;

    Vector deltaPos = distance * moveMatrix.row(2) + strafe * strafeMatrix.row(2);
    _pos -= deltaPos;
}

void Camera::rotate(float ax, float ay)
{
    _angleX += ax;
    _angleY += ay;
}

void Camera::prepare()
{
    Matrix tmp = Matrix::rotateX(_angleX) * Matrix::rotateY(_angleY);
    _matrix = tmp * Matrix::translate( - _pos ) * Matrix::scale(Vector(1,1,-1));
    _matrix.transpose(); // - wtf, why needed ???
}

void Camera::render() const
{
    glLoadMatrixf(_matrix.m);
}
