#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "common.h"
#include "level.h"
#include "body.h"

class Input;
class Game;

class Player : public Collideable
{
public:
    Player(const string& id, const Game* game, const Vector& position, const Vector& rotation);
    virtual ~Player();

    void setDirection(const Vector& direction);
    void setRotation(const Vector& rotation);

    virtual void control(const Input* input) = 0;

    Vector getPosition();

    void onCollision(const NewtonMaterial* material, const NewtonContact* contact);

protected:
    NewtonJoint*        m_upVector;

    bool                m_isOnGround;
    Vector              m_direction;
    Vector              m_rotation;
    const Game*         m_game;

    virtual void onImpact(const Vector& position, const float speed) {} // TODO: remove
    virtual void onScratch(const Vector& position, const float speed) {}

    void onSetForceAndTorque();
    static void onSetForceAndTorque(const NewtonBody* body);

};

#endif
