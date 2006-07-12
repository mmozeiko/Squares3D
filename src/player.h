#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "body.h"

class Input;
class Game;

class Player : public Body
{
public:
    Player(Game* game, int material, const Vector& pos,const Vector& size);
    virtual ~Player();

    void setDirection(const Vector& direction);
    void setRotation(const Vector& rotation);

    virtual void control(const Input* input) = 0;

    void render(const Video* video) const;

    void onCollision(const NewtonMaterial* material, const NewtonContact* contact);

protected:
    NewtonJoint*    m_upVector;

    Vector          m_radius;
    bool            m_isOnGround;
    Vector          m_direction;
    Vector          m_rotation;
    
    unsigned int    m_texture;

    void onSetForceAndTorque();

};

#endif
