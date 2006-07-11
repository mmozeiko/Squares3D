#include "body.h"
#include "common.h"
#include "video.h"

Body::Body(const NewtonWorld* world, const BodyType type) : m_world(world), m_body(NULL), m_type(type)
{
}

Body::~Body()
{
    if (m_body != NULL)
    {
        NewtonReleaseCollision(m_world, m_collision);
        NewtonDestroyBody(m_world, m_body);
    }
}

void Body::create(const NewtonCollision* collision, const Matrix& matrix)
{
    m_collision = collision;
    m_body = NewtonCreateBody(m_world, m_collision);

    NewtonBodySetMatrix(m_body, matrix.m);

    NewtonBodySetUserData(m_body, static_cast<void*>(this));
    NewtonBodySetForceAndTorqueCallback(m_body, onSetForceAndTorque);

}

void Body::prepare()
{
    NewtonBodyGetMatrix(m_body, m_matrix.m);
}

void Body::render(const Video* video) const
{
    video->beginObject(m_matrix);
    onRender(video);
    video->endObject();
}

Body::BodyType Body::getType() const
{
    return m_type;
}

void Body::onSetForceAndTorque()
{
}

void Body::onSetForceAndTorque(const NewtonBody* body)
{
    Body* self = static_cast<Body*>(NewtonBodyGetUserData(body));
    self->onSetForceAndTorque();
}
