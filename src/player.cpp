#include <cmath>

#include "player.h"
#include "video.h"
#include "world.h"
#include "referee.h"
#include "input.h"
#include "xml.h"
#include "collision.h"
#include "character.h"

Player::Player(const Character* character) :
    m_lowerLeft(Vector::Zero),
    m_upperRight(Vector::Zero),
    m_isOnGround(true),
    m_referee(NULL),
    m_character(character)
{
    // set the viscous damping the minimum
    NewtonBodySetLinearDamping(m_character->m_body->m_newtonBody, 0.0f);
    NewtonBodySetAngularDamping(m_character->m_body->m_newtonBody, Vector::Zero.v);

      // add an up vector constraint to help in keeping the body upright
    m_upVector = NewtonConstraintCreateUpVector(World::instance->m_newtonWorld, 
                                                Vector::Y.v, 
                                                m_character->m_body->m_newtonBody); 

    m_character->m_body->setCollideable(this);
}

void Player::setDisplacement(const Vector& position, const Vector& rotation)
{
    m_character->m_body->setTransform(position, rotation);

    float x = FIELDLENGTH * position[0] / abs(position[0]);
    float z = FIELDLENGTH * position[2] / abs(position[2]);
    
    if (x > 0) m_upperRight[0] = x;
    else m_lowerLeft[0] = x;

    if (z > 0) m_upperRight[2] = z;
    else m_lowerLeft[2] = z;
}

Player::~Player()
{
    NewtonDestroyJoint(World::instance->m_newtonWorld, m_upVector);
}

Vector Player::getPosition() const
{
    return m_character->m_body->getPosition();
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
    NewtonBodyGetVelocity(m_character->m_body->m_newtonBody, currentVel.v);
      
    Vector targetVel = m_direction;

    //Vector Force = ( 0.5f * mass * ( ( desiredVel - currentVel ) / timeStep ) ); 
    Vector force = (targetVel * 5.0f - currentVel ) * timestepInv * m_character->m_body->getMass();

    m_isOnGround = fabs(currentVel.y) < 0.01f; // small speed magnitude
    // TODO: move to player_local, at least KEY_SPACE part
    if (!Input::instance->key(GLFW_KEY_SPACE) || !m_isOnGround)
    {
       force.y = 0.0f;
    }

    NewtonBodyAddForce(m_character->m_body->m_newtonBody, force.v);

    m_isOnGround = false;
  
    Vector omega;
    NewtonBodyGetOmega(m_character->m_body->m_newtonBody, omega.v);

    Vector torque = m_rotation;
    torque = (10.0f * torque - omega) * timestepInv * m_character->m_body->getInertia().y;
    NewtonBodyAddTorque (m_character->m_body->m_newtonBody, torque.v);
}

void Player::onCollide(const Body* other, const NewtonMaterial* material)
{
    if (m_referee != NULL)
    {
        m_referee->process(m_character->m_body, other);
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
