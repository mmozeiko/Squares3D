#ifndef __PLAYER_REMOTE_H__
#define __PLAYER_REMOTE_H__

#include "common.h"
#include "player.h"

class ControlPacket;

class RemotePlayer : public Player
{
public:
    RemotePlayer(const Profile* profile, Level* level);
    ~RemotePlayer();

    void control();
    void control(const ControlPacket& packet);

private:
    Vector m_netDirection;
    Vector m_netRotation;
    bool   m_netJump;
    bool   m_netKick;
};

#endif
