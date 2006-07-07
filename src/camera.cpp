#include "camera.h"

#include <GL/glfw.h>

Camera::Camera(const Vector& pos, float angleX, float angleY) :
    _pos(-pos), _angleX(angleX), _angleY(angleY),
    _strafeRotation(Matrix::rotateY(-M_PI/2)),
    _scaleMatrix(Matrix::scale(Vector(1.0f, 1.0f, -1.0f)))
{
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
/*
    glPushMatrix();
    glLoadIdentity();
    glRotatef(_angleX*180.0/M_PI, 1, 0, 0);
    glRotatef(_angleY*180.0/M_PI, 0, 1, 0);
    glTranslatef(_pos.x, _pos.y, _pos.z);
    glScalef(1, 1, -1);
    glGetFloatv(GL_MODELVIEW_MATRIX, _matrix.m);
    glPopMatrix();
*/
    _matrix = Matrix::rotateX(_angleX) * Matrix::rotateY(_angleY) * 
              Matrix::translate( _pos ) * _scaleMatrix;
}

void Camera::Render() const
{
    glLoadMatrixf(_matrix.m);
}
