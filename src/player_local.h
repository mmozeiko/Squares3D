#ifndef __PLAYER_LOCAL_H__
#define __PLAYER_LOCAL_H__

#include "common.h"
#include "player.h"

class Input;

class LocalPlayer : public Player
{
public:
    LocalPlayer(const string& id, const Game* game, const Vector& position, const Vector& rotation);
    ~LocalPlayer();

    void control(const Input* input);

private:
    IntPair m_lastMouse;

};

#endif
