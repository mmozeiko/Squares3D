#include "player_local.h"

#include <GL/glfw.h>

LocalPlayer::LocalPlayer(const NewtonWorld* world, int material, 
        const Vector& pos, const Vector& size) :
    Player(world, material, pos, size)
{
}

LocalPlayer::~LocalPlayer()
{
}

void LocalPlayer::Control()
{
    Vector force;

    if (glfwGetKey('W')==GLFW_PRESS) force.x =  5.0f;
    if (glfwGetKey('S')==GLFW_PRESS) force.x = -5.0f;
    if (glfwGetKey('A')==GLFW_PRESS) force.z =  5.0f;
    if (glfwGetKey('D')==GLFW_PRESS) force.z = -5.0f;

    SetForce(force);
}
