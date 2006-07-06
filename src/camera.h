#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "vmath.h"

#define LOOK_SPEED 20.0f
#define MOVE_SPEED 8.0f

class Camera
{
public:
    Camera(const Vector& pos = Vector(6.0f, 8.0f, 8.0f), float angleX = M_PI/5, float angleY = -M_PI/5);

    void move(float distance, float strafe);
    void rotate(float ax, float ay);

    void prepare();
    void render() const;

private:
    Vector _pos;
    float  _angleX;
    float  _angleY;

    const Matrix _strafeRotation;
    Matrix _matrix;
};
#endif
