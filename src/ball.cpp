#include <GL/glfw.h>
#include <Newton.h>

#include "ball.h"
#include "referee.h"
#include "collision.h"
#include "world.h"
#include "video.h"

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

Ball::Ball(Body* body) : m_body(body)
{
    m_body->setCollideable(this);
    setPosition0();

    NewtonCollision* ballCollision = (*m_body->m_collisions.begin())->m_newtonCollision;

    static const float t = 1.20f; // 20%
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

void Ball::setPosition0()
{
    return m_body->setTransform(Vector(0,2,0), Vector::Zero);
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
    if (foundInMap(m_filteredBodies, other))
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
    static bool first = true;
    static unsigned int list;
    if (first)
    {
        list = Video::instance->newList();
        glNewList(list, GL_COMPILE);

        glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);

        glColor4f(0.2f, 0.2f, 0.2f, 0.2f);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_SRC_ALPHA);

        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, 0.0f);
        float radius = 0.2f;            // TODO: take ball radius from xml file
        for (int i=16; i>=0; i--)
        {
            glVertex3f(radius*cosf(i*2.0f*M_PI/16.0f), 0.0f, radius*sinf(i*2.0f*M_PI/16.0f));
        }
        glEnd();
        glPopAttrib();
        glEndList();
        first = false;
    }

    Vector delta = lightPosition - m_body->getPosition();
    float t = lightPosition.y / delta.y;

    float x = lightPosition.x - t * delta.x;
    float z = lightPosition.z - t * delta.z;

    glPushMatrix();
    glTranslatef(x, 0.01f, z);
    glCallList(list);
    glPopMatrix();
}
