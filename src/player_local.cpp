#include "player_local.h"
#include "input.h"
#include "camera.h"
#include "world.h"
#include "config.h"
#include "network.h"
#include "packet.h"

LocalPlayer::LocalPlayer(const Profile* profile, Level* level) :
    Player(profile, level), m_cpacket(NULL) //Config::instance->m_misc.mouse_sensitivity)
{
}

LocalPlayer::~LocalPlayer()
{
    if (m_cpacket != NULL)
    {
        delete m_cpacket;
    }
}

void LocalPlayer::control()
{
    const Mouse& mouse = Input::instance->mouse();
    Vector curMouse(static_cast<float>(mouse.x), 0.0f, static_cast<float>(mouse.y));
    if ((mouse.b & 2) == 2)
    {
        setDirection(Vector::Zero);
        return;
    }
    Vector direction = curMouse;

    direction.z = -direction.z;
    direction /= 8.9f;

    direction = 0.8f * direction + 0.15f * m_lastMove[1] + 0.05f * m_lastMove[0];

    m_lastMove[0] = m_lastMove[1];
    m_lastMove[1] = direction;

    //this disables acceleration
    //direction.norm();

    Vector finalDirection = Matrix::rotateY(World::instance->m_camera->angleY()) * direction;

    Body* ball = World::instance->m_level->getBody("football");

    Vector ballPosition = ball->getPosition();
    Vector selfPosition = m_body->getPosition();

    Vector dir = ballPosition - selfPosition;
    Vector rot = m_body->getRotation();
    Vector rotation;
    rotation.y = ( rot % dir );

    bool jump = Input::instance->key(GLFW_KEY_SPACE);
    bool kick = (mouse.b & 1) == 1;
    rotation /= 5.0f;

    if (Network::instance->m_isSingle || Network::instance->m_isServer)
    {
        setDirection(finalDirection);
        setJump(jump);
        setRotation(rotation);
        setKick(kick);
    }
    else
    {
        if (m_cpacket != NULL)
        {
            delete m_cpacket;
        }
        
        m_cpacket = new ControlPacket(Network::instance->getLocalIdx(), finalDirection, rotation, jump, kick);
    }
}

void LocalPlayer::control(const ControlPacket& packet)
{
    clog << "LocalPlayer::control - invalid call" << endl;
}

ControlPacket* LocalPlayer::getControl(int idx)
{
    return m_cpacket;
}