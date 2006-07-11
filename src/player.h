#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "body.h"

class Input;

class Player : public Body
{
public:
    Player(const NewtonWorld* world, int material, 
        const Vector& pos,
        const Vector& size);
    virtual ~Player();

    void setForce(const Vector& force);

    virtual void control(const Input* input) = 0;
    void onRender(const Video* video) const;

    void onCollision(const NewtonMaterial* material, const NewtonContact* contact);

private:
    NewtonJoint*    m_upVector;

    Vector          m_radius;
    bool            m_isOnGround;
    Vector          m_force;
   
    float           m_angleY;

    void onSetForceAndTorque();

};

#endif
