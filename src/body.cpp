#include "body.h"
#include "collision.h"
#include "game.h"
#include "xml.h"
#include "world.h"
#include "video.h"
#include "level.h"
#include "properties.h"
#include "geometry.h"

Body::Body(const string& id, const Collision* collision):
    m_id(id),
    m_matrix(),
    m_collideable(NULL)
{
    m_totalMass = collision->m_mass;
    m_totalInertia = collision->m_inertia;

    m_collisions.insert(collision);

    createNewtonBody(collision->m_newtonCollision, collision->m_origin, Vector(), Vector());
}    

Body::Body(const XMLnode& node):
    m_matrix(),
    m_totalMass(0.0f),
    m_totalInertia(0.0f, 0.0f, 0.0f),
    m_collideable(NULL)
{
    NewtonCollision* newtonCollision = NULL;
    m_id = node.getAttribute("id");
    
    Vector position(0.0f, 0.0f, 0.0f);
    Vector rotation(0.0f, 0.0f, 0.0f);

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "position")
        {
            position = node.getAttributesInVector("xyz");
        }
        else if (node.name == "rotation")
        {
            rotation = node.getAttributesInVector("xyz") * DEG_IN_RAD;
        }
        else if (node.name == "collision")
        { 
            m_collisions.insert(World::instance->m_level->getCollision(node.value));
        }
        else
        {
            throw Exception("Invalid body, unknown node - " + node.name);
        }
    }

    Vector totalOrigin(0.0f, 0.0f, 0.0f);

    if (m_collisions.size() == 1)
    {
        const Collision* collision = *m_collisions.begin();

        m_totalMass = collision->m_mass;
        m_totalInertia = collision->m_inertia;
        totalOrigin = collision->m_origin;

        newtonCollision = collision->m_newtonCollision;
    }
    else if (m_collisions.size() > 1)
    {
        vector<NewtonCollision*> newtonCollisions(m_collisions.size());
        int cnt = 0;
        for each_const(CollisionSet, m_collisions, iter)
        {
            const Collision* collision = *iter;
            newtonCollisions[cnt++] = collision->m_newtonCollision;
            m_totalMass += collision->m_mass;
            m_totalInertia += collision->m_inertia;
            totalOrigin += collision->m_origin;
        }
        totalOrigin /= m_totalMass;

        newtonCollision = NewtonCreateCompoundCollision(
                                                World::instance->m_newtonWorld,
                                                cnt,
                                                &newtonCollisions[0]);
        for each_const(vector<NewtonCollision*>, newtonCollisions, collision)
        {
            //NewtonReleaseCollision(World::instance->m_newtonWorld, *collision);
        }
    }
    else
    {
        throw Exception("No collisions were found for body '" + m_id + "'");
    }

    createNewtonBody(newtonCollision, totalOrigin, position, rotation);
}

void Body::setKickForce(const Vector& force)
{
    m_kickForce = force;
}

void Body::setTransform(const Vector& position, const Vector& rotation)
{
    NewtonSetEulerAngle(rotation.v, m_matrix.m);
    m_matrix = Matrix::translate(position) * m_matrix;
            
    NewtonBodySetMatrix(m_newtonBody, m_matrix.m);
}

void Body::createNewtonBody(const NewtonCollision* newtonCollision,
                            const Vector&          totalOrigin,
                            const Vector&          position,
                            const Vector&          rotation)
{
    m_newtonBody = NewtonCreateBody(World::instance->m_newtonWorld, newtonCollision);
    NewtonBodySetUserData(m_newtonBody, static_cast<void*>(this));

    setTransform(position, rotation);

    if (m_id != "level" && m_id != "wall")
    {
        NewtonBodySetMassMatrix(m_newtonBody, m_totalMass, m_totalInertia.x, m_totalInertia.y, m_totalInertia.z);
        NewtonBodySetCentreOfMass(m_newtonBody, totalOrigin.v);
        NewtonBodySetAutoFreeze(m_newtonBody, 0);
        NewtonBodySetForceAndTorqueCallback(m_newtonBody, onSetForceAndTorque);
    }

    // danger - ball depends on newtonCollision variable
    //NewtonReleaseCollision(World::instance->m_newtonWorld, newtonCollision);
}

Body::~Body()
{
    //TODO: destroy m_body

    /*
    for each_const(CollisionSet, m_collisions, iter)
    {
        NewtonReleaseCollision(World::instance->m_newtonWorld, (*iter)->m_newtonCollision);
    }
    */
}

void Body::prepare()
{
    NewtonBodyGetMatrix(m_newtonBody, m_matrix.m);
    NewtonBodyGetVelocity(m_newtonBody, m_velocity.v);
}

Vector Body::getPosition() const
{
    return m_matrix.row(3);
}

Vector Body::getRotation() const
{
    return m_matrix.row(0);
}

Vector Body::getVelocity() const
{
    return m_velocity;
}

void Body::onSetForceAndTorque()
{
    Vector force = gravityVec * m_totalMass;
    NewtonBodySetForce(m_newtonBody, force.v);
    
    if (m_kickForce != Vector::Zero)
    {
        NewtonBodySetVelocity(m_newtonBody, Vector::Zero.v);
        NewtonBodyAddForce(m_newtonBody, m_kickForce.v);
        m_kickForce = Vector::Zero;
    }

    if (m_collideable != NULL)
    {
        m_collideable->onSetForceAndTorque();
    }
}

void Body::onSetForceAndTorque(const NewtonBody* body)
{
    Body* self = static_cast<Body*>(NewtonBodyGetUserData(body));
    self->onSetForceAndTorque();
}

void Body::render() const
{
    bool doit = false;
    if (Video::instance->m_shadowMap3ndPass)
    {
        const Vector pos = m_matrix.row(3);
        if (!isPointInRectangle(pos, g_fieldLower, g_fieldUpper))
        {
            Video::glActiveTextureARB(GL_TEXTURE1_ARB);
            glDisable(GL_TEXTURE_2D);
            Video::glActiveTextureARB(GL_TEXTURE0_ARB);
            doit = true;
        }
    }

    Video::instance->begin(m_matrix);

    for each_const(CollisionSet, m_collisions, iter)
    {
        (*iter)->render();
    }

    if (doit)
    {
        Video::glActiveTextureARB(GL_TEXTURE1_ARB);
        glEnable(GL_TEXTURE_2D);
        Video::glActiveTextureARB(GL_TEXTURE0_ARB);
    }

    Video::instance->end();
}

void Body::setCollideable(Collideable* collideable)
{
    m_collideable = collideable;
}

void Body::onCollide(const Body* other, const NewtonMaterial* material)
{
    if (m_collideable != NULL)
    {
        m_collideable->onCollide(other, material);
    }
}

void Body::onCollideHull(const Body* other, const NewtonMaterial* material)
{
    if (m_collideable != NULL)
    {
        m_collideable->onCollideHull(other, material);
    }
}

void Body::onImpact(const Body* other, const Vector& position, float speed)
{
    if (m_collideable != NULL)
    {
        m_collideable->onImpact(other, position, speed);
    }
}

void Body::onScratch(const Body* other, const Vector& position, float speed)
{
    if (m_collideable != NULL)
    {
        m_collideable->onScratch(other, position, speed);
    }
}

float Body::getMass() const
{
    return m_totalMass;
}

const Vector& Body::getInertia() const
{
    return m_totalInertia;
}
