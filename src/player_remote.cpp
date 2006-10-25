#include "player_remote.h"
#include "world.h"
#include "body.h"
#include "random.h"
#include "geometry.h"
#include "packet.h"
#include "xml.h"

RemotePlayer::RemotePlayer(const Profile* profile, Level* level) :
    Player(profile, level),
    m_netDirection(),
    m_netRotation(),
    m_netJump(false),
    m_netKick(false)
{
}

RemotePlayer::~RemotePlayer()
{
}

void RemotePlayer::control()
{
    // use these from status
    setDirection(m_netDirection);
    setRotation(m_netRotation);
    setJump(m_netJump);
    setJump(m_netKick);
    
    m_netDirection /= 2.0f;
    m_netRotation /= 2.0f;
}

void RemotePlayer::control(const ControlPacket& packet)
{
    // update status from packet
    m_netDirection = packet.m_netDirection;
    m_netRotation = packet.m_netRotation;
    m_netJump = packet.m_netJump;
    m_netKick = packet.m_netKick;
}
