#include "player_ai.h"
#include "game.h"
#include "input.h"
#include "world.h"
#include "body.h"

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

    Vector direction = ball->getPosition() - getPosition();
    direction.norm();

    //Vector rotation = m_body->getRotation();



    setDirection(direction);
}
