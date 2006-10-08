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
    m_jump(false),
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
    m_direction.norm();
    // CHARACTER: move speed powaaaar
    m_direction *= 2.2f;
}

void Player::setRotation(const Vector& rotation)
{
    // CHARACTER: rotate speed powaaar
    m_rotation = rotation;
    m_rotation *= 2.2f;
}

void Player::onSetForceAndTorque()
{
    float timestepInv = 1.0f / NewtonGetTimeStep(World::instance->m_newtonWorld);

    Vector currentVel;
    NewtonBodyGetVelocity(m_character->m_body->m_newtonBody, currentVel.v);
      
    const Vector targetVel = m_direction;
    Vector force = 0.5f * (targetVel - currentVel ) * timestepInv * m_character->m_body->getMass();
    
    if (m_jump && m_isOnGround)
    {
        // CHARACTER: jump powaaar!! not higher that 1.5f
        force.y = 1.0f * timestepInv * m_character->m_body->getMass();
        m_isOnGround = false;
    }
    else
    {
        force.y = 0.0f;
    }

    NewtonBodyAddForce(m_character->m_body->m_newtonBody, force.v);
 

    Vector omega;
    NewtonBodyGetOmega(m_character->m_body->m_newtonBody, omega.v);

    const Vector targetOmega = m_rotation;
    Vector torque = 0.5f * (targetOmega - omega) * timestepInv * m_character->m_body->getInertia().y;
    NewtonBodyAddTorque (m_character->m_body->m_newtonBody, torque.v);
}

void Player::onCollide(const Body* other, const NewtonMaterial* material)
{
    if (m_referee != NULL)
    {
        m_referee->process(m_character->m_body, other);
    }
    m_isOnGround = true;
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

void Player::setJump(bool needJump)
{
    m_jump = needJump;
}
