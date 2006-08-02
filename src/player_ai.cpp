#include "player_ai.h"
#include "game.h"
#include "input.h"
#include "world.h"
#include "body.h"
#include "camera.h"
#include "random.h"

AiPlayer::AiPlayer(const string& id, const Game* game, const Vector& position, const Vector& rotation) :
    Player(id, game, position, rotation)
{

}

AiPlayer::~AiPlayer()
{
}

void AiPlayer::control(const Input* input)
{

    Body* ball = m_game->m_world->m_level->getBody("football");

    Vector rot = m_body->getRotation();
    Vector dir = ball->getPosition() - m_body->getPosition();
    float r1 = static_cast<float>(rand())/RAND_MAX - 0.5f;
    float r2 = static_cast<float>(rand())/RAND_MAX - 0.5f;
    dir += Vector(1.0f*r1, 0.0f, 1.0f*r2);
    dir.y = 0;
    dir.norm();
    Vector rotation;

    rotation.y = ( rot % dir ) / 3.5f;
    

    Vector direction(-rot.z, 0, rot.x);
    direction.norm();
    if (m_body->getPosition().y > 0.15f)
    {
        direction *= 1.5f;
    }
  
    setDirection(direction);
    setRotation(rotation);
}
