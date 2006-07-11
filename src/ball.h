#ifndef __BALL_H__
#define __BALL_H__

#include "body.h"

class Game;

class Ball : public Body
{
public:
    Ball(const NewtonWorld* world, const Vector& pos, Game* game, const float radius = 1.0f);
    ~Ball();

    void onRender(const Video* video) const;

private:
    unsigned int m_texture;
    float m_radius;
    void onSetForceAndTorque();

};

#endif
