#include "body.h"
#include "common.h"
#include "video.h"
#include "game.h"
#include "world.h"

Body::Body(Game* game) :
    Renderable(game),
    m_body(NULL)
{
    m_world = m_game->m_world->m_world;
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

	// disable auto freeze
	NewtonBodySetAutoFreeze(m_body, 0);
	NewtonWorldUnfreezeBody(m_world, m_body);

    NewtonBodySetUserData(m_body, static_cast<void*>(this));
    NewtonBodySetForceAndTorqueCallback(m_body, onSetForceAndTorque);

}

void Body::update(float delta)
{
}

void Body::prepare()
{
    NewtonBodyGetMatrix(m_body, m_matrix.m);
}

void Body::onSetForceAndTorque()
{
}

void Body::onSetForceAndTorque(const NewtonBody* body)
{
    Body* self = static_cast<Body*>(NewtonBodyGetUserData(body));
    self->onSetForceAndTorque();
}
