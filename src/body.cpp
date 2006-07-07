#include "body.h"
#include "common.h"

Body::Body(const NewtonWorld* world, const BodyType type) : _world(world), _body(NULL), _type(type)
{
}

Body::~Body()
{
    if (_body != NULL)
    {
        NewtonReleaseCollision(_world, _collision);
        NewtonDestroyBody(_world, _body);
    }
}

void Body::Create(const NewtonCollision* collision, const Matrix& matrix)
{
    _collision = collision;
    _body = NewtonCreateBody(_world, _collision);

    NewtonBodySetMatrix(_body, matrix.m);

    NewtonBodySetUserData(_body, static_cast<void*>(this));
    NewtonBodySetForceAndTorqueCallback(_body, onSetForceAndTorque);

}

void Body::Prepare()
{
    NewtonBodyGetMatrix(_body, _matrix.m);
}

void Body::Render(const Video& video) const
{
    video.BeginObject(_matrix);
    onRender(video);
    video.EndObject();
}

Body::BodyType Body::GetType() const
{
    return _type;
}

void Body::onSetForceAndTorque()
{
}

void Body::onSetForceAndTorque(const NewtonBody* body)
{
    Body* self = static_cast<Body*>(NewtonBodyGetUserData(body));
    self->onSetForceAndTorque();
}
