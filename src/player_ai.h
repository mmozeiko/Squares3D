#ifndef __PLAYER_AI_H__
#define __PLAYER_AI_H__

#include "common.h"
#include "player.h"

class Input;

class AiPlayer : public Player
{
public:
    AiPlayer(const XMLnode& node, const Level* level);
    ~AiPlayer();

    void control();
};

#endif
