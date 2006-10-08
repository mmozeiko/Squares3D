#include "player_local.h"
#include "input.h"
#include "camera.h"
#include "world.h"
#include "config.h"

LocalPlayer::LocalPlayer(const Profile* profile, const Character* character, Level* level) :
    Player(profile, character, level) //Config::instance->m_misc.mouse_sensitivity)
{
    int w, h; 
    glfwGetWindowSize(&w, &h);
    m_lastMouse = Vector(static_cast<float>(w/2), static_cast<float>(h/2));

}

LocalPlayer::~LocalPlayer()
{
}

void LocalPlayer::control()
{
    const Mouse& mouse = Input::instance->mouse();
    Vector curMouse(static_cast<float>(mouse.x), 0.0f, static_cast<float>(mouse.y));

    const float alpha = 0.8f;

    Vector direction = curMouse - m_lastMouse;
    direction.z = -direction.z;
    direction /= 8.9f;

    m_lastMouse = curMouse;
    
    //this disables acceleration
    //direction.norm();

    setDirection(Matrix::rotateY(World::instance->m_camera->angleY()) * direction);
    setJump(Input::instance->key(GLFW_KEY_SPACE));

    Body* ball = World::instance->m_level->getBody("football");

    Vector ballPosition = ball->getPosition();
    Vector selfPosition = m_body->getPosition();

    Vector dir = ballPosition - selfPosition;
    Vector rot = m_body->getRotation();
    Vector rotation;
    rotation.y = ( rot % dir );

    setRotation(rotation);
}
