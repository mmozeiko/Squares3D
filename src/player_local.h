#ifndef __PLAYER_LOCAL_H__
#define __PLAYER_LOCAL_H__

#include "common.h"
#include "player.h"

class Input;

class LocalPlayer : public Player
{
public:
    LocalPlayer(const XMLnode& node, const Level* level);
    ~LocalPlayer();

    void control();

private:
    IntPair m_lastMouse;

};

#endif
