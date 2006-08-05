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
    Vector center;
    if (lowerLeft[0] < 0) center[0] = lowerLeft[0] / 2;
    else center[0] = upperRight[0] / 2;

    if (lowerLeft[2] < 0) center[2] = lowerLeft[2] / 2;
    else center[2] = upperRight[2] / 2;
    return center;
}
//def getBallAndSquareIntersection(position, velocity, min, max):
//  adjust = 0.0f #how far can we exceed own square border
//
//  min0 = min[0]
//  min1 = min[1]
//  max0 = max[0]
//  max1 = max[1]
//
//  
//  t = position[2] / (velocity[2] + 1e-06)
//  if t >= 0:
//    x = position[0] + velocity[0] * t
//    if x > min0 and x < max0:
//      return (x, 0.0, 0.0)
//  t = position[0] / (velocity[0] + 1e-06)
//  if t >= 0:
//    y = position[2] + velocity[2] * t
//    if y > min1 and y < max1:
//      return (0.0, 0.0, y)
//  return ((max[0] + min[0])/2, 0.0, (max[1] + min[1])/2)

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
    Vector squareCenter = getSquareCenter(m_lowerLeft, m_upperRight);

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
        direction = squareCenter - selfPosition;
    }

    direction.norm();

    if (m_body->getPosition().y > 0.15f)
    {
        direction *= 1.5f;
    }

    setDirection(direction / 1.5f);
    setRotation(rotation);
}