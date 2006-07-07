#ifndef __BALL_H__
#define __BALL_H__

#include "body.h"

class Ball : public Body
{
public:
    Ball(const NewtonWorld* world, const Vector& pos, const float radius = 1.0f);
    ~Ball();

    void onRender(const Video& video) const;

private:
    float _radius;
    void onSetForceAndTorque();

};

#endif
