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

private:
    Vector  m_lastMove[2];
    Vector  m_lastMouse;
    float   m_mouseSens;

};

#endif
