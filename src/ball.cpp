#include <Newton.h>

#include "ball.h"
#include "referee.h"
#include "collision.h"
#include "world.h"

Ball::Ball(Body* body) : m_body(body)
{
    m_body->setCollideable(this);

    NewtonCollision* ballCollision = (*m_body->m_collisions.begin())->m_newtonCollision;
    NewtonCollision* hull = NewtonCreateConvexHullModifier(World::instance->m_newtonWorld, ballCollision);

    // hull scale - 10%
    static const float t = 1.10f;
    Matrix matrix = Matrix::scale(Vector(t, t, t));

    NewtonConvexHullModifierSetMatrix(hull, matrix.m);
    // TODO: get invisible id (1) from real m_properties
    NewtonConvexCollisionSetUserID(hull, 1); // m_properties->getInvisible()

    NewtonCollision* both[] = { ballCollision, hull };
    NewtonCollision* newCollision = NewtonCreateCompoundCollision(World::instance->m_newtonWorld, sizeOfArray(both), both);

    NewtonBodySetCollision(m_body->m_newtonBody, newCollision);

    // TODO: hmm?
    //NewtonReleaseCollision(hull);
    //NewtonReleaseCollision(newCollision);
}

Vector Ball::getPosition()
{
    return m_body->getPosition();
}

void Ball::setPosition0()
{
    return m_body->setTransform(Vector(0,2,0), Vector(0,0,0));
}

void Ball::onCollide(Body* other, const NewtonMaterial* material)
{
    m_referee->process(m_body, other);
}

void Ball::onCollideHull(Body* other, const NewtonMaterial* material)
{
    // collision with hull
    1;
}
