#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "common.h"
#include "vmath.h"

class Camera
{
public:
    Camera(const Vector& pos = Vector(15.0f, 20.0f, 15.0f), float angleX = 0, float angleY = 0);
    ~Camera();  

    void control();
    void update(float delta);
    void prepare();
    void render() const;

    float angleY() const;

private:
    IntPair m_lastMouse;

    Vector m_targetRotation;
    Vector m_targetDirection;

    Vector m_pos;
    float  m_angleX;
    float  m_angleY;

    const Matrix m_strafeRotation;
    const Matrix m_scaleMatrix;
    Matrix m_matrix;
};

#endif
