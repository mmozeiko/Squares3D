#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <Newton.h>
#include "vmath.h"
#include "body.h"

class Player : public Body
{
public:
    Player(const NewtonWorld* world, int material, 
        const Vector& pos,
        const Vector& size);
    virtual ~Player();

    void SetForce(const Vector& force);

    virtual void Control() = 0;
    void onRender(const Video& video) const;

    void onCollision(const NewtonMaterial* material, const NewtonContact* contact);

private:
    NewtonJoint*        _upVector;

    Vector              _radius;
    bool                _stopped;
    float               _maxStepHigh;
    Vector              _force;
    
    float               _angleY;

    void onSetForceAndTorque();

};

#endif