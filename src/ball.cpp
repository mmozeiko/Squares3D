#include <Newton.h>

#include "ball.h"
#include "referee.h"
#include "collision.h"
#include "world.h"

TriggerFlags::TriggerFlags()
{
    loadDefaults();
}

void TriggerFlags::loadDefaults()
{
    m_wasTriggeredBefore = false;
    m_shouldRegisterCollision = false;
}

Ball::Ball(Body* body) : 
    m_body(body)
{
    m_body->setCollideable(this);
    m_body->setTransform(Vector(0, 2, 0), Vector::Zero);

    NewtonCollision* ballCollision = (*m_body->m_collisions.begin())->m_newtonCollision;

    // TODO: read size from xml
    static const float t = 0.22f;
    NewtonCollision* hull = NewtonCreateSphere(World::instance->m_newtonWorld, t, t, t, NULL);
    // TODO: get invisible id (1) from real m_properties
    NewtonConvexCollisionSetUserID(hull, 1); // m_properties->getInvisible()

    NewtonCollision* both[] = { ballCollision, hull };
    NewtonCollision* newCollision = NewtonCreateCompoundCollision(World::instance->m_newtonWorld, sizeOfArray(both), both);
    NewtonBodySetCollision(m_body->m_newtonBody, newCollision);
    NewtonConvexCollisionSetUserID(newCollision, NewtonConvexCollisionGetUserID(ballCollision));

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

void Ball::addBodyToFilter(const Body* body)
{
    m_filteredBodies[body] = TriggerFlags();
}

void Ball::onCollide(const Body* other, const NewtonMaterial* material)
{
    if (!foundInMap(m_filteredBodies, other))
    {
        m_referee->process(m_body, other);
    }
}

void Ball::onCollideHull(const Body* other, const NewtonMaterial* material)
{
    // when trigger collides
    if (!foundInMap(m_filteredBodies, other))
    {
        return;
    }

    //for bodies in filter map
    m_hasTriggered = true;
    TriggerFlags& triggerFlags = m_filteredBodies.find(other)->second;
    if (!triggerFlags.m_wasTriggeredBefore)
    {
        //if trigger has just collided with the opposing body
        triggerFlags.m_shouldRegisterCollision = true;
        triggerFlags.m_wasTriggeredBefore = true;
    }
}

void Ball::triggerBegin()
{
    m_hasTriggered = false;
}

void Ball::triggerEnd()
{
    // body is in m_filteredBodies set
    if (!m_hasTriggered)
    {
        for each_(TriggerFilterMap, m_filteredBodies, iter)
        {
            iter->second.loadDefaults();
        }
    }
    else
    {
        for each_(TriggerFilterMap, m_filteredBodies, iter)
        {
            TriggerFlags& triggerFlags = iter->second;
            if (triggerFlags.m_shouldRegisterCollision)
            {
                m_referee->process(m_body, iter->first);
                triggerFlags.m_shouldRegisterCollision = false;
            }
        }
    }
}
