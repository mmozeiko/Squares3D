#include <Newton.h>

#include "ball.h"
#include "referee.h"
#include "collision.h"
#include "world.h"

Ball::Ball(Body* body) : 
    m_body(body),
    m_wasTriggeredBefore(false),
    m_hasCollidedWithBall(false),
    m_shouldRegisterCollision(false)
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
    NewtonConvexCollisionSetUserID(hull, NewtonConvexCollisionGetUserID(ballCollision));

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
    if (!foundInSet<const Body*>(m_filteredBodies, other))
    {
        m_referee->process(m_body, other);
    }
}

void Ball::onCollideHull(Body* other, const NewtonMaterial* material)
{
    if (!foundInSet<const Body*>(m_filteredBodies, other))
    {
        return;
    }

    // when trigger collides
    if (!m_hasCollidedWithBall)
    {
        m_wasTriggeredBefore;
        m_shouldRegisterCollision = true;
    }
}

void Ball::triggerBegin()
{
    //m_wasTriggeredBefore = false;
    //m_hasCollidedWithBall = false;
}

void Ball::triggerEnd()
{
            // body is in m_filteredBodies set
    //if (foundInSet<const Body*>(m_filteredBodies, other))
    {
        //if (m_shouldRegisterCollision)
        {
            //m_referee->process(m_body, other);
            //m_shouldRegisterCollision = false;
            //m_hasCollidedWithBall = true;
        }
    }

}


    //This is the main logic function of the coach
    //Coach has 3 states to keep in mind when registering event
    //States are needed to avoid registering ball+player event when
    //the ball is standing on player not bouncing
    //This is the place where TriggerBox is used

