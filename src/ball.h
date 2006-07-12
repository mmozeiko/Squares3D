#ifndef __BALL_H__
#define __BALL_H__

#include "body.h"

class Game;

class Input;
class Ball : public Body
{
public:
    Ball(Game* game, const Vector& pos, const float radius = 1.0f);
    ~Ball();

    void render(const Video* video) const;

private:
    unsigned int m_texture;
    float m_radius;

    void onSetForceAndTorque();
    void control(const Input*);
};

#endif
