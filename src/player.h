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
    void render(const Video& video);

    void OnCollision(const NewtonMaterial* material, const NewtonContact* contact);

private:
    const NewtonWorld*  _world;
    NewtonBody*         _body;
    NewtonCollision*    _collision;
    NewtonJoint*        _upVector;

    Vector              _radius;
    Vector              _stepContact;
    
    bool                _stopped;
    bool                _inAir;
    int                 _jumpTimer;
    float               _maxStepHigh;
    float               _maxTranslation;

    Vector              _force;
    Matrix              _matrix;

    float findFloor(const Vector& centre, float maxDist);

    void OnApplyForce();

    static float OnRayCast(const NewtonBody* body, const float* normal, int collisionID, void* userData, float intersetParam);
    static void OnApplyForce(const NewtonBody* body);

};

#endif
