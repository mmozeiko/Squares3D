#ifndef __PLAYER_LOCAL_H__
#define __PLAYER_LOCAL_H__

#include "common.h"
#include "player.h"

class LocalPlayer : public Player
{
public:
    LocalPlayer(const Character* character);
    ~LocalPlayer();

    void control();

private:
    IntPair m_lastMouse;

};

#endif
