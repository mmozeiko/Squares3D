#include "player_local.h"

#include "input.h"

#include <GL/glfw.h>

LocalPlayer::LocalPlayer(const NewtonWorld* world, int material, 
        const Vector& pos, const Vector& size) :
    Player(world, material, pos, size)
{
}

LocalPlayer::~LocalPlayer()
{
}

void LocalPlayer::control(const Input* input)
{
    Vector force;

    if (input->key('W')) force.x =  5.0f;
    if (input->key('S')) force.x = -5.0f;
    if (input->key('A')) force.z =  5.0f;
    if (input->key('D')) force.z = -5.0f;

    setForce(force);
}
