#ifndef __PLAYER_LOCAL_H__
#define __PLAYER_LOCAL_H__

#include "common.h"
#include "player.h"

class LocalPlayer : public Player
{
public:
    LocalPlayer(const Profile* profile, Level* level);
    ~LocalPlayer();

    void control();
    void control(const ControlPacket& packet);
    ControlPacket* getControl(int idx);

private:
    Vector         m_lastMove[2];
    float          m_mouseSens;
    ControlPacket* m_cpacket;

};

#endif
