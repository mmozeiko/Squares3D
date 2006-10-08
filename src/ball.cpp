#include <GL/glfw.h>
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

    m_shadowList = Video::instance->newList();
    glNewList(m_shadowList, GL_COMPILE);

    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC_ALPHA);
    glColor4f(0.2f, 0.2f, 0.2f, 0.1f);

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
    const Vector pos = m_body->getPosition();
    const Vector lp(lightPosition.x, lightPosition.y*2.0f, lightPosition.z);
    const Vector delta = lp - pos;
    float t = lp.y / delta.y;

    float x = lp.x - t * delta.x;
    float z = lp.z - t * delta.z;

    float y;

    if (isPointInRectangle(pos, Vector(-3, 0, -3), Vector(3, 0, 3)))
    {
        y = 0.0105f;
    }
    else
    {
        y = m_levelCollision->getHeight(pos.x, pos.z) + 0.01f;
        x = pos.x;
        z = pos.z;
    }

    glPushMatrix();
    glTranslatef(x, y, z);
    glCallList(m_shadowList);
    glPopMatrix();
}
