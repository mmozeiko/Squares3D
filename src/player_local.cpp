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

void LocalPlayer::control()
{
    Vector force;

    if (glfwGetKey('W')==GLFW_PRESS) force.x =  50.0f;
    if (glfwGetKey('S')==GLFW_PRESS) force.x = -50.0f;
    if (glfwGetKey('A')==GLFW_PRESS) force.z =  50.0f;
    if (glfwGetKey('D')==GLFW_PRESS) force.z = -50.0f;

    setForce(force);
}
