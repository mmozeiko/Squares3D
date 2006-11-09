#include <cmath>
#include <Newton.h>

#include "player.h"
#include "video.h"
#include "world.h"
#include "referee_local.h"
#include "input.h"
#include "xml.h"
#include "collision.h"
#include "profile.h"
#include "ball.h"
#include "properties.h"
#include "level.h"
#include "packet.h"
#include "network.h"

static const pair<float, float> jumpMinMax = make_pair(0.7f, 1.0f);
static const pair<float, float> speedMinMax = make_pair(2.5f, 4.5f);
static const pair<float, float> rotateSpeedMinMax = make_pair(8.0f, 14.0f);
static const pair<float, float> accuracyMinMax = make_pair(0.2f, 1.0f);

float _applyCoefficient(const pair<float, float> minMax, float const coefficient)
{
    return (minMax.second - minMax.first) * coefficient + minMax.first;
}


Player::Player(const Profile* profile, Level* level) :
    m_referee(NULL),
    m_body(NULL),
    m_profile(profile),
    m_lowerLeft(Vector::Zero),
    m_upperRight(Vector::Zero),
    m_upVector(NULL),
    m_isOnGround(true),
    m_jump(false),
    m_halt(false),
    m_levelCollision(level->getCollision("level")),
    m_ballBody(level->getBody("football")),
    m_packet(NULL)
{
    CollisionSet collisions;
    collisions.insert(level->getCollision(m_profile->m_collisionID));
    m_body = new Body(m_profile->m_name, level, collisions);
    level->m_bodies[m_profile->m_name] = m_body;
    
    m_radius = (*collisions.begin())->getRadius();
    
    // set the viscous damping the minimum
    NewtonBodySetLinearDamping(m_body->m_newtonBody, 0.0f);
    NewtonBodySetAngularDamping(m_body->m_newtonBody, Vector::Zero.v);

      // add an up vector constraint to help in keeping the body upright
    m_upVector = NewtonConstraintCreateUpVector(World::instance->m_newtonWorld, 
                                                Vector::Y.v, 
                                                m_body->m_newtonBody); 

    m_body->setCollideable(this);

    //extract the player coefficients

    m_speedCoefficient = _applyCoefficient(speedMinMax, m_profile->m_speed);
    m_rotateSpeedCoefficient = _applyCoefficient(rotateSpeedMinMax, m_profile->m_speed);
    m_accuracyCoefficient = _applyCoefficient(accuracyMinMax, m_profile->m_accuracy);
    m_jumpCoefficient = _applyCoefficient(jumpMinMax, m_profile->m_jump);
}

void Player::setPositionRotation(const Vector& position, const Vector& rotation)
{
    m_body->setTransform(position, rotation);

    float x = FIELD_LENGTH * position[0] / std::abs(position[0]);
    float z = FIELD_LENGTH * position[2] / std::abs(position[2]);
    
    if (x > 0) m_upperRight[0] = x;
    else m_lowerLeft[0] = x;

    if (z > 0) m_upperRight[2] = z;
    else m_lowerLeft[2] = z;
}

Player::~Player()
{
    NewtonDestroyJoint(World::instance->m_newtonWorld, m_upVector);
    if (m_packet != NULL)
    {
        delete m_packet;
    }
}

void Player::setKick(bool needKick)
{
    m_kick = needKick;
    
    if (m_kick && (Network::instance->m_isServer || Network::instance->m_isSingle))
    {
        Vector dist = m_ballBody->getPosition() - m_body->getPosition();
        
        if ( m_timer.read() > 0.4f && dist.magnitude2() < (0.2f+BALL_RADIUS+m_radius)*(0.2f+BALL_RADIUS+m_radius) ) // TODO: (gurkja resnums + bumbas_raadiuss)^2
        {
            Properties* prop = World::instance->m_level->m_properties;
            const pair<byte, SoundBuffer*>* sb = prop->getSB(prop->getPropertyID("player"), prop->getPropertyID("football"));
            prop->play(m_body, sb, true, m_ballBody->getPosition());
            Network::instance->addSoundPacket(sb->first, m_ballBody->getPosition());

            // CHARACTER: kick powaaar!
            Vector f = m_ballBody->getPosition() - m_body->getPosition();
            f.norm();
            f.y = 4.0f;
            f.norm();
            m_ballBody->setKickForce(500.0f * f); //Vector(0, 500, 0)); // 400-700
            m_timer.reset();
            m_referee->addDelayedProcess(m_ballBody, m_body, 0.1f);
        }
    }
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
    m_direction = 0.8f * m_speedCoefficient * direction + 0.2f * m_direction;
}

void Player::setRotation(const Vector& rotation)
{
    m_rotation = 0.8f * m_rotateSpeedCoefficient * rotation + 0.2f * m_rotation;
}

void Player::onSetForceAndTorque()
{
    float timestepInv = 1.0f / NewtonGetTimeStep(World::instance->m_newtonWorld);

    Vector currentVel;
    NewtonBodyGetVelocity(m_body->m_newtonBody, currentVel.v);

    Vector targetVel = m_direction;
    
    // to avoid high speed movemements
    bool highY = false;
    if (fabsf(currentVel.x) > 2.0f) targetVel.x = 0.0f; //-currentVel.x;
    if (currentVel.y > 3.0f)
    {
        highY = true;
        targetVel.y = 0.0f; //-currentVel.y/4.0f;
    }
    if (fabsf(currentVel.z) > 2.0f) targetVel.z = 0.0f; //-currentVel.z;

    Vector force = 0.5f * (targetVel - currentVel ) * timestepInv * m_body->getMass();
    
    if (m_jump && m_isOnGround)
    {
        if (currentVel.y < 3.0f) // to avoid n-jumps (n>2)
        {
            // CHARACTER: jump powaaar!! not higher that 1.5f - 2.5f
            force.y = 3.0f * timestepInv * m_body->getMass();
        }
        m_isOnGround = false;
    }
    else
    {
        if (!highY)
        {
            force.y = 0.0f;
        }
    }

    NewtonBodyAddForce(m_body->m_newtonBody, force.v);
 

    Vector omega;
    NewtonBodyGetOmega(m_body->m_newtonBody, omega.v);

    const Vector targetOmega = m_rotation;
    Vector torque = 0.5f * (targetOmega - omega) * timestepInv * m_body->getInertia().y;
    NewtonBodyAddTorque(m_body->m_newtonBody, torque.v);
}

void Player::onCollide(const Body* other, const NewtonMaterial* material, const Vector& position, float speed)
{
    if (m_referee != NULL)
    {
        m_referee->process(m_body, other);
    }
    m_isOnGround = true;

    Vector v;
    NewtonBodyGetVelocity(other->m_newtonBody, v.v);
}

void Player::setJump(bool needJump)
{
    m_jump = needJump;
}

void Player::renderColor() const
{
    Vector c(m_profile->m_color);
    c.w = 0.3f;
    Video::instance->renderSimpleShadow(0.3f, m_body->getPosition(), m_levelCollision, c);
}

ControlPacket* Player::getControl(int idx)
{
    assert(false);
    return NULL;
}
