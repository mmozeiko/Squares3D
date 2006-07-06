#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <Newton.h>
#include "vmath.h"

class Video;

class Player
{
public:
    Player(const NewtonWorld* world, int material, 
        const Vector& pos,
        const Vector& size);
    virtual ~Player();

    void setForce(const Vector& force);

    virtual void control() = 0;
    virtual void prepare();
    void render(const Video& video) const;

    void OnCollision(const NewtonMaterial* material, const NewtonContact* contact);

private:
    const NewtonWorld*  _world;
    NewtonBody*         _body;
    NewtonJoint*        _upVector;

    Vector              _radius;
    
    bool                _stopped;
    float               _maxStepHigh;

    Vector              _force;
    Matrix              _matrix;

    void OnApplyForce();

    static void OnApplyForce(const NewtonBody* body);

};

#endif
