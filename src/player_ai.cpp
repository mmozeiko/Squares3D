#include "player_ai.h"
#include "input.h"
#include "world.h"
#include "body.h"
#include "camera.h"
#include "random.h"
#include "geometry.h"

AiPlayer::AiPlayer(const string& id, const Vector& position, const Vector& rotation) :
    Player(id, position, rotation)
{
}

AiPlayer::~AiPlayer()
{
}

void AiPlayer::control()
{
    Body* ball = World::instance->m_level->getBody("football");

    Vector ballPosition = ball->getPosition();
    Vector selfPosition = m_body->getPosition();

    bool move = true;

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

        important = false;
    }

    Vector dir = ballPosition - selfPosition;

    if (dir.magnitude() < 0.6f && !important)
    {
        move = false;
    }
    else
    {
        float r1 = static_cast<float>(rand())/RAND_MAX - 0.5f;
        float r2 = static_cast<float>(rand())/RAND_MAX - 0.5f;
        //dir += Vector(1.0f*r1, 0.0f, 1.0f*r2);
        dir.y = 0;
        dir.norm();
    }

    Vector rot = m_body->getRotation();

    Vector rotation;
    if (move)
    {
        rotation.y = ( rot % dir );
    }
    else
    {
        Vector tmp = (ball->getPosition()-selfPosition);
        tmp.norm();
        rotation.y = ( rot % tmp );
    }
    
    rotation.y /= 1.0f; // rotation speed

    Vector direction(-rot.z, 0, rot.x);

    direction.norm();

    // what does this if do?? a?
    if (m_body->getPosition().y > 0.15f)
    {
        direction *= 1.5f;
    }

    if (move)
    {
        setDirection(direction / 3.5f); // move speed
    }
    else
    {
        setDirection(Vector::Zero); // i'm in position!
    }

    setRotation(rotation);
}