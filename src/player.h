#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "common.h"
#include "level.h"
#include "body.h"

class Input;
class Game;
class Referee;

class Player : public Collideable
{
public:
    Player(const string& id, const Game* game, const Vector& position, const Vector& rotation);
    virtual ~Player();

    void setDirection(const Vector& direction);
    void setRotation(const Vector& rotation);

    virtual void control(const Input* input) = 0;

    Vector getPosition();

    void onCollide(Body* other, const NewtonMaterial* material);
    
	Referee* m_referee;

protected:
    NewtonJoint*        m_upVector;

    bool                m_isOnGround;
    Vector              m_direction;
    Vector              m_rotation;
    const Game*         m_game;
    Body*               m_body;

    void onSetForceAndTorque();
    static void onSetForceAndTorque(const NewtonBody* body);

};

#endif
