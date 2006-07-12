#ifndef __PLAYER_LOCAL_H__
#define __PLAYER_LOCAL_H__

#include "player.h"
#include "common.h"

class Input;

class LocalPlayer : public Player
{
public:
    LocalPlayer(Game* game, int material, const Vector& pos, const Vector& size);
    ~LocalPlayer();

    void control(const Input* input);

private:
    IntPair m_lastMouse;
};

#endif
