#include <cmath>

#include "player.h"
#include "video.h"
#include "world.h"
#include "referee.h"
#include "input.h"
#include "xml.h"
#include "collision.h"
#include "level.h"

Player::Player(const XMLnode& node, const Level* level) :
    m_lowerLeft(Vector::Zero),
    m_upperRight(Vector::Zero),
    m_isOnGround(true),
    m_referee(NULL)
{
    m_name = node.getAttribute("id");

    Collision* collision = level->getCollision(node.getAttribute("collisionID"));
    
    m_body = new Body(m_name, collision);

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "color")
        {
            m_color = getAttributesInVector(node, "rgb");
        }
        else if (node.name == "char")
        {
            m_speed = node.getAttribute<float>("speed");
            m_accuracy = node.getAttribute<float>("accuracy");
        }
        else
        {
            throw Exception("Invalid player, unknown node - " + node.name);
        }
    }
    
    // set the viscous damping the minimum
    NewtonBodySetLinearDamping(m_body->m_newtonBody, 0.0f);
    NewtonBodySetAngularDamping(m_body->m_newtonBody, Vector::Zero.v);

      // add an up vector constraint to help in keeping the body upright
    m_upVector = NewtonConstraintCreateUpVector(World::instance->m_newtonWorld, Vector::Y.v, m_body->m_newtonBody); 

    m_body->setCollideable(this);
}

void Player::setDisplacement(const Vector& position, const Vector& rotation)
{
    m_body->setTransform(position, rotation);

    float x = FIELDLENGTH * position[0] / abs(position[0]);
    float z = FIELDLENGTH * position[2] / abs(position[2]);
    
    if (x > 0) m_upperRight[0] = x;
    else m_lowerLeft[0] = x;

    if (z > 0) m_upperRight[2] = z;
    else m_lowerLeft[2] = z;
}

Player::~Player()
{
    delete m_body;
    NewtonDestroyJoint(World::instance->m_newtonWorld, m_upVector);
}

Vector Player::getPosition() const
{
    return m_body->getPosition();
}

Vector Player::getFieldCenter() const
{
    Vector sum = m_lowerLeft + m_upperRight;
    return Vector(sum.x / 2, 0, sum.z / 2);
}

void Player::setDirection(const Vector& direction)
{
    m_direction = direction;
}

void Player::setRotation(const Vector& rotation)
{
    m_rotation = rotation;
}

void Player::onSetForceAndTorque()
{
    float timestepInv = 1.0f / NewtonGetTimeStep(World::instance->m_newtonWorld);

    Vector currentVel;
    NewtonBodyGetVelocity(m_body->m_newtonBody, currentVel.v);
      
    Vector targetVel = m_direction;

    Vector force = (targetVel * 5.0f - currentVel ) * timestepInv * m_body->getMass();

    m_isOnGround = fabs(currentVel.y) < 0.01f; // small speed magnitude
    // TODO: move to player_local, at least KEY_SPACE part
    if (!Input::instance->key(GLFW_KEY_SPACE) || !m_isOnGround)
    {
       force.y = 0.0f;
    }

    NewtonBodyAddForce(m_body->m_newtonBody, force.v);

    m_isOnGround = false;
  
    Vector omega;
    NewtonBodyGetOmega(m_body->m_newtonBody, omega.v);

    Vector torque = m_rotation;
    torque = (10.0f * torque - omega) * timestepInv * m_body->getInertia().y;
    NewtonBodyAddTorque (m_body->m_newtonBody, torque.v);
}

void Player::onCollide(const Body* other, const NewtonMaterial* material)
{
    if (m_referee != NULL)
    {
        m_referee->process(m_body, other);
    }
}

void Player::onImpact(const Body* other, const Vector& position, const float speed)
{
    Vector v;
    NewtonBodyGetVelocity(other->m_newtonBody, v.v);
}

void Player::onScratch(const Body* other, const Vector& position, const float speed)
{
    onImpact(other, position, speed);
}
