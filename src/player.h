#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "level.h"

class Input;
class Game;
class World;

class Player
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
    World*              m_world;
    LevelObjects::Body* m_body;

    void onSetForceAndTorque();

    static void onSetForceAndTorque(const NewtonBody* body);

};

#endif
