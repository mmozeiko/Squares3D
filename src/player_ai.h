#ifndef __PLAYER_AI_H__
#define __PLAYER_AI_H__

#include "player.h"
#include "common.h"

class Input;

class AiPlayer : public Player
{
public:
    AiPlayer(const string& id, const Game* game, const Vector& position, const Vector& rotation);
    ~AiPlayer();

    void control(const Input* input);

private:

};

#endif
