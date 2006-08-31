#include "body.h"
#include "collision.h"
#include "game.h"
#include "xml.h"
#include "world.h"
#include "video.h"
#include "level.h"
#include "properties.h"

typedef map<string, Collision*> CollisionsMap;
typedef map<string, Body*>      BodiesMap;

Body::Body(const XMLnode& node, const Game* game):
    m_matrix(),
    m_totalMass(0.0f),
    m_totalInertia(0.0f, 0.0f, 0.0f),
    m_newtonWorld(game->m_world->m_newtonWorld),
    m_collideable(NULL)
{
    NewtonCollision* newtonCollision = NULL;
    string id = getAttribute(node, "id");
    
    int materialID = -1;
    StringMap::const_iterator iter = node.attributes.find("property");
    if (iter != node.attributes.end())
    {
        materialID = game->m_world->m_level->m_properties->getPropertyID(iter->second);
    }

    Vector position(0.0f, 0.0f, 0.0f);
    Vector rotation(0.0f, 0.0f, 0.0f);

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "position")
        {
            position = getAttributesInVector(node, "xyz");
        }
        else if (node.name == "rotation")
        {
            rotation = getAttributesInVector(node, "xyz") * DEG_IN_RAD;
        }
        else if (node.name == "collision")
        { 
            string id = node.value;
            CollisionsMap::const_iterator iter = game->m_world->m_level->m_collisions.find(id);
            if (iter != game->m_world->m_level->m_collisions.end())
            {
                m_collisions.insert(iter->second);
            }
            else
            {
                throw Exception("Could not find specified collision for body '" + id + "'");
            }
        }
        else
        {
            throw Exception("Invalid body, unknown node - " + node.name);
        }
    }

    Vector totalOrigin(0.0f, 0.0f, 0.0f);

    if (m_collisions.size() == 1)
    {
        Collision* collision = *m_collisions.begin();

        m_totalMass = collision->m_mass;
        m_totalInertia = collision->m_inertia;
        totalOrigin = collision->m_origin;

        newtonCollision = collision->m_newtonCollision;
    }
    else if (m_collisions.size() > 1)
    {
        vector<NewtonCollision*> newtonCollisions(m_collisions.size());
        int cnt = 0;
        for each_const(set<Collision*>, m_collisions, iter)
        {
            Collision* collision = *iter;
            newtonCollisions[cnt++] = collision->m_newtonCollision;
            m_totalMass += collision->m_mass;
            m_totalInertia += collision->m_inertia;
            totalOrigin += collision->m_origin;
        }
        totalOrigin /= m_totalMass;

        newtonCollision = NewtonCreateCompoundCollision(
                                                m_newtonWorld,
                                                cnt,
                                                &newtonCollisions[0]);
        for each_const(vector<NewtonCollision*>, newtonCollisions, collision)
        {
            NewtonReleaseCollision(m_newtonWorld, *collision);
        }
    }
    else
    {
        throw Exception("No collisions were found for body '" + id + "'");
    }

    createNewtonBody(newtonCollision, totalOrigin, position, rotation, materialID);

    if (id == "football")
    {
        NewtonBodySetContinuousCollisionMode(m_newtonBody, 1);
    }
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
                            const Vector&          rotation,
                            const int              materialID)
{
    m_newtonBody = NewtonCreateBody(m_newtonWorld, newtonCollision);
    NewtonBodySetUserData(m_newtonBody, static_cast<void*>(this));

    if (materialID != -1)
    {
        NewtonBodySetMaterialGroupID(m_newtonBody, materialID);
    }

    NewtonBodySetMassMatrix(m_newtonBody, m_totalMass, m_totalInertia.x, m_totalInertia.y, m_totalInertia.z);
    NewtonBodySetCentreOfMass(m_newtonBody, totalOrigin.v);
    
    NewtonBodySetForceAndTorqueCallback(m_newtonBody, onSetForceAndTorque);

    setTransform(position, rotation);

    NewtonBodySetAutoFreeze(m_newtonBody, 0);

    NewtonReleaseCollision(m_newtonWorld, newtonCollision);
}

Body::~Body()
{
    /*
    for each_const(set<Collision*>, m_collisions, iter)
    {
        NewtonReleaseCollision(m_newtonWorld, (*iter)->m_newtonCollision);
    }
    */
}

void Body::prepare()
{
    NewtonBodyGetMatrix(m_newtonBody, m_matrix.m);
}

Vector Body::getPosition()
{
    return m_matrix.row(3);
}

Vector Body::getRotation()
{
    //Vector angles;
    //NewtonGetEulerAngle(m_matrix.m, angles.v);
    return m_matrix.row(0);
}

void Body::onSetForceAndTorque()
{
    Vector force = gravityVec * m_totalMass;
    NewtonBodyAddForce(m_newtonBody, force.v);
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

void Body::render(const Video* video)
{
    video->begin(m_matrix);

    for each_const(set<Collision*>, m_collisions, iter)
    {
        (*iter)->render(video);
    }

    video->end();
}

void Body::setCollideable(Collideable* collideable)
{
    m_collideable = collideable;
}

void Body::onCollide(Body* other, const NewtonMaterial* material)
{
    if (m_collideable != NULL)
    {
        m_collideable->onCollide(other, material);
    }
}

void Body::onImpact(Body* other, const Vector& position, const float speed)
{
    if (m_collideable != NULL)
    {
        m_collideable->onImpact(other, position, speed);
    }
}

void Body::onScratch(Body* other, const Vector& position, const float speed)
{
    if (m_collideable != NULL)
    {
        m_collideable->onScratch(other, position, speed);
    }
}
