#include "player_ai.h"
#include "game.h"
#include "input.h"

AiPlayer::AiPlayer(const string& id, const Game* game, const Vector& position, const Vector& rotation) :
    Player(id, game, position, rotation)
{

}

AiPlayer::~AiPlayer()
{
}

void AiPlayer::control(const Input* input)
{

    Player* humanPlayer = m_world->m_localPlayers[0];

    Vector direction = humanPlayer->getPosition() - getPosition();
    
    direction.norm();
    
    direction.x /=  2.0f;
    direction.z /=  2.0f;

    setDirection(direction);
}
