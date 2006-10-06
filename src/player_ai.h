#ifndef __PLAYER_AI_H__
#define __PLAYER_AI_H__

#include "common.h"
#include "player.h"

class Input;

class AiPlayer : public Player
{
public:
    AiPlayer(const Character* character);
    ~AiPlayer();

    void control();
};

#endif
