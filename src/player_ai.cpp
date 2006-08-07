#include "player_ai.h"
#include "game.h"
#include "input.h"
#include "world.h"
#include "body.h"
#include "camera.h"
#include "random.h"

#define FIELDLENGTH 3.0f

//    |-----|max
//    |     |
// min|-----|
bool isBallInSquare(const Vector& ballPosition, const Vector& lowerLeft, const Vector& upperRight)
{
  if ((ballPosition[0] >= lowerLeft[0]) 
      && (ballPosition[0] <= upperRight[0])
      && (ballPosition[2] >= lowerLeft[2]) 
      && (ballPosition[2] <= upperRight[2]))
  {
      return true;
  }
  return false;
}

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

    float t = 0.0f, x = 0.0f, z = 0.0f;
    bool intersectsX = false, intersectsZ = false;

    Vector squareCenter = getSquareCenter(lowerLeft, upperRight);
    Vector result(squareCenter), intersectionX, intersectionZ;

    t = position[2] / static_cast<float>(velocity[2] + 1e-06);
    
    if (t <= 0) 
    {
        x = position[0] - velocity[0] * t;
        if (((x > min0) && (x < max0)) 
            && ((getQuadrant(position)) != (getQuadrant(squareCenter)))) 
        {
            intersectionX = Vector(x, 0.0f, 0.0f);
            intersectsX = true;
        }
    }

    t = position[0] / static_cast<float>(velocity[0] + 1e-06);

    if (t <= 0) 
    {
        z = position[2] - velocity[2] * t;
        if (((z > min2) && (z < max2)) 
            && ((getQuadrant(position)) != (getQuadrant(squareCenter))))
        {
            intersectionZ = Vector(0.0f, 0.0f, z);
            intersectsZ = true;
        }
    }

    if (intersectsX && intersectsZ)
    {
        if ((intersectionX - position).magnitude() < (intersectionZ - position).magnitude())
        {
            result = intersectionX;
        }
        else { result = intersectionZ; }
    }
    else if (intersectsX) { result = intersectionX; }
    else if (intersectsZ) { result = intersectionZ; }

    return result;
}

AiPlayer::AiPlayer(const string& id, const Game* game, const Vector& position, const Vector& rotation) :
    Player(id, game, position, rotation)
{
    float x = FIELDLENGTH * position[0] / abs(position[0]);
    float z = FIELDLENGTH * position[2] / abs(position[2]);
    
    if (x > 0) m_upperRight[0] = x;
    else m_lowerLeft[0] = x;

    if (z > 0) m_upperRight[2] = z;
    else m_lowerLeft[2] = z;
}

AiPlayer::~AiPlayer()
{
}

void AiPlayer::control(const Input* input)
{

    Body* ball = m_game->m_world->m_level->getBody("football");

    Vector ballPosition = ball->getPosition();
    Vector selfPosition = m_body->getPosition();

    Vector rot = m_body->getRotation();
    Vector dir = ballPosition - selfPosition;
    float r1 = static_cast<float>(rand())/RAND_MAX - 0.5f;
    float r2 = static_cast<float>(rand())/RAND_MAX - 0.5f;
    dir += Vector(1.0f*r1, 0.0f, 1.0f*r2);
    dir.y = 0;
    dir.norm();
    Vector rotation;

    rotation.y = ( rot % dir ) / 1.0f;

    Vector direction(-rot.z, 0, rot.x);

    if (!isBallInSquare(ballPosition, m_lowerLeft, m_upperRight))
    {
        Vector ballVelocity;
        NewtonBodyGetVelocity(ball->m_newtonBody, ballVelocity.v);

        Vector intersection = findBallAndSquareIntersection(
                                                  ballPosition, 
                                                  ballVelocity, 
                                                  m_lowerLeft, 
                                                  m_upperRight);
        direction = intersection - selfPosition;
    }

    direction.norm();

    if (m_body->getPosition().y > 0.15f)
    {
        direction *= 1.5f;
    }

    setDirection(direction / 1.5f);
    setRotation(rotation);
}