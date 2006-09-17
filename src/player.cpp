#include <cmath>

#include "player.h"
#include "video.h"
#include "world.h"
#include "referee.h"
#include "input.h"

Player::Player(const string& id, const Vector& position, const Vector& rotation) :
    m_body(World::instance->m_level->getBody(id)),
    m_isOnGround(true)
{
    m_body->setTransform(position, rotation);

    float x = FIELDLENGTH * position[0] / abs(position[0]);
    float z = FIELDLENGTH * position[2] / abs(position[2]);
    
    if (x > 0) m_upperRight[0] = x;
    else m_lowerLeft[0] = x;

    if (z > 0) m_upperRight[2] = z;
    else m_lowerLeft[2] = z;

    // set the viscous damping the minimum
    NewtonBodySetLinearDamping(m_body->m_newtonBody, 0.0f);
    NewtonBodySetAngularDamping(m_body->m_newtonBody, Vector::Zero.v);

      // add an up vector constraint to help in keeping the body upright
    m_upVector = NewtonConstraintCreateUpVector(World::instance->m_newtonWorld, Vector::Y.v, m_body->m_newtonBody); 

    m_body->setCollideable(this);
}

Player::~Player()
{
    NewtonDestroyJoint(World::instance->m_newtonWorld, m_upVector);
}

Vector Player::getPosition()
{
    return m_body->getPosition();
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
    m_referee->process(m_body, other);
}

void Player::onImpact(const Body* other, const Vector& position, const float speed)
{
    Vector v;
    NewtonBodyGetVelocity(other->m_newtonBody, v.v);
    m_isOnGround = fabs(v.y) < 0.01f; // small speed magnitude
}

void Player::onScratch(const Body* other, const Vector& position, const float speed)
{
    onImpact(other, position, speed);
}
