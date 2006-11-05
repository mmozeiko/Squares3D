#include <Newton.h>

#include "ball.h"
#include "referee.h"
#include "collision.h"
#include "world.h"
#include "video.h"
#include "geometry.h"

TriggerFlags::TriggerFlags()
{
    loadDefaults();
}

void TriggerFlags::loadDefaults()
{
    m_wasTriggeredBefore = false;
    m_shouldRegisterCollision = false;
    m_hasTriggered = false;
}

Ball::Ball(Body* body, const Collision* levelCollision) : m_body(body), m_levelCollision(levelCollision)
{
    m_body->setCollideable(this);
    setPosition0();
    //NewtonBodySetOmega(m_body->m_newtonBody, Vector(-10, 0, 0).v);

    NewtonCollision* ballCollision = (*m_body->m_collisions.begin())->m_newtonCollision;

    static const float t = 1.3f; // 30%
    NewtonCollision* hull = NewtonCreateConvexHullModifier(World::instance->m_newtonWorld, ballCollision);
    NewtonConvexHullModifierSetMatrix(hull, Matrix::scale(Vector(t, t, t)).m);
    //NewtonCollision* hull = NewtonCreateSphere(World::instance->m_newtonWorld, t, t, t, NULL);

    // TODO: get invisible id (1) from real m_properties
    NewtonConvexCollisionSetUserID(hull, 1); // m_properties->getInvisible()

    NewtonCollision* both[] = { ballCollision, hull };
    NewtonCollision* newCollision = NewtonCreateCompoundCollision(World::instance->m_newtonWorld, sizeOfArray(both), both);
    NewtonConvexCollisionSetUserID(newCollision, NewtonConvexCollisionGetUserID(ballCollision));
    NewtonBodySetCollision(m_body->m_newtonBody, newCollision);

    // TODO: hmm?
    //NewtonReleaseCollision(hull);
    //NewtonReleaseCollision(newCollision);
}

Vector Ball::getPosition() const
{
    return m_body->getPosition();
}

Vector Ball::getVelocity() const
{
    return m_body->getVelocity();
}

void Ball::setPosition0()
{
    return m_body->setTransform(Vector(0,2,0), Vector::Zero);
}

void Ball::addBodyToFilter(const Body* body)
{
    m_filteredBodies[body] = TriggerFlags();
}

void Ball::onCollide(const Body* other, const NewtonMaterial* material, const Vector& position, float speed)
{
    if (m_referee->isGroundObject(other))
    {
        if (!((other->m_id == "field") && m_referee->m_playersAreHalted))
        {
            //if it`s not the very special case when players wait for the
            //ball bounce specified times - register
            m_referee->process(m_body, other);
        }
    }
}

void Ball::onCollideHull(const Body* other)
{
    // when trigger collides
    if (foundIn(m_filteredBodies, other))
    {
        //for bodies in filter map
        TriggerFlags& triggerFlags = m_filteredBodies[other];
        triggerFlags.m_hasTriggered = true;
        if (!triggerFlags.m_wasTriggeredBefore)
        {
            //if trigger has just collided with the opposing body
            triggerFlags.m_shouldRegisterCollision = true;
            triggerFlags.m_wasTriggeredBefore = true;
        }
    }
}

void Ball::triggerBegin()
{
    for each_(TriggerFilterMap, m_filteredBodies, iter)
    {
        iter->second.m_hasTriggered = false;
    }
}

void Ball::triggerEnd()
{
    // body is in m_filteredBodies set
    for each_(TriggerFilterMap, m_filteredBodies, iter)
    {
        TriggerFlags& triggerFlags = iter->second;
        if (!triggerFlags.m_hasTriggered)
        {
            triggerFlags.loadDefaults();
        }
        else
        {
            if (triggerFlags.m_shouldRegisterCollision)
            {
                m_referee->process(m_body, iter->first);
                triggerFlags.m_shouldRegisterCollision = false;
            }
        }
    }
}

void Ball::renderShadow(const Vector& lightPosition) const
{
    Vector pos = m_body->getPosition();

    if (isPointInRectangle(pos, Vector(-3, 0, -3), Vector(3, 0, 3)))
    {
        const Vector lp(lightPosition.x, lightPosition.y*2.0f, lightPosition.z);
        const Vector delta = lp - pos;
        const float t = lp.y / delta.y;

        pos.x = lp.x - t * delta.x;
        pos.z = lp.z - t * delta.z;
    }

    Video::instance->renderSimpleShadow(0.2f, pos, m_levelCollision, Vector(0.2f, 0.2f, 0.2f, 0.7f));
}
