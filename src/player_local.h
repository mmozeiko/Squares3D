#ifndef __PLAYER_LOCAL_H__
#define __PLAYER_LOCAL_H__

#include "player.h"

class Input;

class LocalPlayer : public Player
{
public:
    LocalPlayer(const NewtonWorld* world, int material, 
        const Vector& pos,
        const Vector& size);
    ~LocalPlayer();

    void control(const Input* input);
private:

};

#endif
