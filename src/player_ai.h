#ifndef __PLAYER_AI_H__
#define __PLAYER_AI_H__

#include "common.h"
#include "player.h"

class Input;

class AiPlayer : public Player
{
public:
    AiPlayer(const Profile* profile, Level* level);
    ~AiPlayer();

    void halt();
    void release();

    void control();
    void control(const ControlPacket& packet);
};

#endif
