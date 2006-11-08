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
}

void RemotePlayer::control(const ControlPacket& packet)
{
    // update status from packet
    setDirection(packet.m_netDirection);
    setRotation(packet.m_netRotation);
    setJump(packet.m_netJump);
    setKick(packet.m_netKick);
}
