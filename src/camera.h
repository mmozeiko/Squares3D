#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "vmath.h"
#include "common.h"
#include "renderable.h"

class Camera : public Renderable
{
public:
    Camera(Game* game, const Vector& pos = Vector(6.0f, 8.0f, 8.0f), float angleX = M_PI/5, float angleY = -M_PI/5);
    ~Camera();  

    void control(const Input* input);
    void update(float delta);
    void prepare();
    void render(const Video* video) const;

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
