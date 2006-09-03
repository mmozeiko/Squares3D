#ifndef __BALL_H__
#define __BALL_H__

#include "body.h"

class Game;
class Referee;

class Ball : public Collideable
{
public:
    Ball(Body* body, const Game* game);

    Vector getPosition();
    void   setPosition0();

    // maybe private
    void onCollide(Body* other, const NewtonMaterial* material);
    void onCollideHull(Body* other, const NewtonMaterial* material);
    
    Referee*            m_referee;
    Body*               m_body;

protected:
    const Game*         m_game;
};

#endif
