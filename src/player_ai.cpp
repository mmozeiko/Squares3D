#include "player_ai.h"
#include "input.h"
#include "world.h"
#include "body.h"
#include "random.h"
#include "geometry.h"
#include "level.h"

AiPlayer::AiPlayer(const Profile* profile, Level* level) :
    Player(profile, level)
{
}

void AiPlayer::halt()
{
    //halt those englander bastards!
    m_halt = true;
}

void AiPlayer::release() 
{
    //ok you can go..
    m_halt = false;
}

AiPlayer::~AiPlayer()
{
}

void AiPlayer::control()
{
    Body* ball = World::instance->m_level->getBody("football");

    Vector ballPosition = ball->getPosition();
    const Vector selfPosition = m_body->getPosition();

    bool move = !m_halt;

    bool important = true;

    if (!isPointInRectangle(ballPosition, m_lowerLeft, m_upperRight))
    {
        Vector ballVelocity;
        NewtonBodyGetVelocity(ball->m_newtonBody, ballVelocity.v);

        ballPosition = findBallAndSquareIntersection(
            ballPosition, 
            ballVelocity, 
            m_lowerLeft, 
            m_upperRight);

        important = (getFieldCenter() - ballPosition).magnitude() >= 1.0f;
    }

    // important if going to hit ball, else going to center of players field

    Vector dir = ballPosition - selfPosition;

    if (dir.magnitude() < 0.7f && !important) // if inside players field center
    {
        move = false;
    }
    else
    {
        if (important &&                    // if moving towards ball
            dir.magnitude() < 1.0f &&       // and ball is nearby
            (ball->getVelocity().y > 0 || ball->getPosition().y > m_radius*2) &&    // and ball is going upwards or is above gurkjis
            ball->getPosition().y > 0.3f && // and ball is flying 
            Randoms::getFloat() < m_jumpCoefficient // and very probable random
            )
        {
            setJump(true);
        }
        else
        {
            setJump(false);
        }

        const float acc = 1.0f / m_accuracyCoefficient;

        float r1 = acc*Randoms::getFloat() - acc/2.0f;
        float r2 = acc*Randoms::getFloat() - acc/2.0f;
        dir += Vector(r1, 0.0f, r2);
        dir.y = 0;
        dir.norm();
    }

    Vector rot = m_body->getRotation();

    Vector rotation;
    if (!move)
    {
        dir = (ball->getPosition() - selfPosition);
        dir.norm();
    }
    rotation.y = ( rot % dir );

    setRotation(rotation);
    
    Vector resultDir;
    if (move)
    {
        resultDir = Vector(-rot.z, 0, rot.x);
    }
    else
    {
        resultDir = getFieldCenter() - selfPosition;
    }
    resultDir.norm();

    setDirection(resultDir);
}
