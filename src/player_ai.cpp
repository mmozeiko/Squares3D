#include "player_ai.h"
#include "input.h"
#include "world.h"
#include "body.h"
#include "camera.h"
#include "random.h"

Vector getSquareCenter(const Vector& lowerLeft, const Vector& upperRight)
{
    return Vector((upperRight[0] + lowerLeft[0])/2, 0.0f, (upperRight[2] + lowerLeft[2])/2);
}

unsigned int getQuadrant(const Vector& point)
{
    if ((point[0] <= 0) && (point[2] >= 0))
    {
        return 1;
    }
    else if ((point[0] >= 0) && (point[2] >= 0))
    {
        return 2;
    }
    else if ((point[0] >= 0) && (point[2] <= 0))
    {
        return 3;
    }
    else
    {
        return 4;
    }
}

Vector findBallAndSquareIntersection(const Vector& position, 
                                     const Vector& velocity, 
                                     const Vector& lowerLeft, 
                                     const Vector& upperRight)
{
    //this function returns the center of square if no intersection is found

    float min0 = lowerLeft[0];
    float min2 = lowerLeft[2];
    float max0 = upperRight[0];
    float max2 = upperRight[2];

    float t1 = 0.0f, t2 = 0.0f, x = 0.0f, z = 0.0f;
    bool intersectsX = false, intersectsZ = false;

    Vector squareCenter = getSquareCenter(lowerLeft, upperRight);
    Vector result(squareCenter), intersectionX, intersectionZ;

    t1 = position[2] / static_cast<float>(velocity[2] + 1e-06);
    
    if (t1 <= 0) 
    {
        x = position[0] - velocity[0] * t1;
        if (((x > min0) && (x < max0)) 
            && ((getQuadrant(position)) != (getQuadrant(squareCenter)))) 
        {
            intersectionX = Vector(x, 0.0f, 0.0f);
            intersectsX = true;
        }
    }

    t2 = position[0] / static_cast<float>(velocity[0] + 1e-06);

    if (t2 <= 0) 
    {
        z = position[2] - velocity[2] * t2;
        if (((z > min2) && (z < max2)) 
            && ((getQuadrant(position)) != (getQuadrant(squareCenter))))
        {
            intersectionZ = Vector(0.0f, 0.0f, z);
            intersectsZ = true;
        }
    }

    if (intersectsX && intersectsZ)
    {
        if (t1 > t2)
        {
            result = intersectionX;
        }
        else { result = intersectionZ; }
    }
    else if (intersectsX) { result = intersectionX; }
    else if (intersectsZ) { result = intersectionZ; }

    return result;
}

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

    if (!isPointInRectangle(ballPosition, m_lowerLeft, m_upperRight))
    {
        Vector ballVelocity;
        NewtonBodyGetVelocity(ball->m_newtonBody, ballVelocity.v);

        ballPosition = findBallAndSquareIntersection(
            ballPosition, 
            ballVelocity, 
            m_lowerLeft, 
            m_upperRight);
    }

    Vector dir = ballPosition - selfPosition;

    if (dir.magnitude() < 0.6f)
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
        setDirection(direction / 3.0f); // move speed
    }
    else
    {
        setDirection(Vector::Zero); // i'm in position!
    }

    setRotation(rotation);
}