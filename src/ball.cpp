#include "ball.h"
#include "video.h"

Ball::Ball(const NewtonWorld* world, const Vector& pos, const float radius) :
    Body(world, BallBody), _radius(radius)
{
    NewtonCollision* collision = NewtonCreateSphere(_world, _radius, _radius, _radius, NULL);
    Body::Create(collision, Matrix::translate(pos));

    Vector intertia, origin;
    NewtonConvexCollisionCalculateInertialMatrix(collision, intertia.v, origin.v);
    NewtonBodySetMassMatrix(_body, 1.0f, intertia.x, intertia.y, intertia.z);
}

Ball::~Ball()
{
}

void Ball::onSetForceAndTorque()
{
    NewtonBodyAddForce(_body, gravityVec.v);
}

void Ball::onRender(const Video* video) const
{
    video->RenderSphere(_radius);
}
