#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "vmath.h"

#define LOOK_SPEED 20.0f
#define MOVE_SPEED 8.0f

class Camera
{
public:
    Camera(const Vector& pos = Vector(6.0f, 8.0f, 8.0f), float angleX = M_PI/5, float angleY = -M_PI/5);
    ~Camera();

    void move(float distance, float strafe);
    void rotate(float ax, float ay);

    void control(float delta);
    void prepare();
    void render() const;

private:
    Vector m_pos;
    float  m_angleX;
    float  m_angleY;

    const Matrix m_strafeRotation;
    const Matrix m_scaleMatrix;
    Matrix m_matrix;
};
#endif
