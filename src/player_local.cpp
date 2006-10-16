#include "player_local.h"
#include "input.h"
#include "camera.h"
#include "world.h"
#include "config.h"

LocalPlayer::LocalPlayer(const Profile* profile, Level* level) :
    Player(profile, level) //Config::instance->m_misc.mouse_sensitivity)
{
    const Mouse& mouse = Input::instance->mouse();
    m_lastMouse = Vector(static_cast<float>(mouse.x), 0.0f, static_cast<float>(mouse.y));
}

LocalPlayer::~LocalPlayer()
{
}

void LocalPlayer::control()
{
    const Mouse& mouse = Input::instance->mouse();
    Vector curMouse(static_cast<float>(mouse.x), 0.0f, static_cast<float>(mouse.y));
    if ((mouse.b & 2) == 2)
    {
        m_lastMouse = curMouse;
        setDirection(Vector::Zero);
        return;
    }

    Vector direction = curMouse - m_lastMouse;

    direction.z = -direction.z;
    direction /= 8.9f;

    direction = 0.7f * direction + 0.2f * m_lastMove[1] + 0.1f * m_lastMove[0];

    m_lastMove[0] = m_lastMove[1];
    m_lastMove[1] = direction;
    m_lastMouse = curMouse;
    
    //this disables acceleration
    //direction.norm();

    Vector finalDirection = Matrix::rotateY(World::instance->m_camera->angleY()) * direction;

    setDirection(finalDirection);
    setJump(Input::instance->key(GLFW_KEY_SPACE));

    Body* ball = World::instance->m_level->getBody("football");

    Vector ballPosition = ball->getPosition();
    Vector selfPosition = m_body->getPosition();

    Vector dir = ballPosition - selfPosition;
    Vector rot = m_body->getRotation();
    Vector rotation;
    rotation.y = ( rot % dir );

    setRotation(rotation/5.0f);

    if (mouse.b & 1)
    {
        // .. kick
        setKick(finalDirection);
    }
}
