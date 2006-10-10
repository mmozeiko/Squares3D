#include "player_remote.h"
#include "world.h"
#include "body.h"
#include "random.h"
#include "geometry.h"
#include "packet.h"
#include "xml.h"

RemotePlayer::RemotePlayer(const Profile* profile, Level* level) :
    Player(profile, level)
{
}

RemotePlayer::~RemotePlayer()
{
}

void RemotePlayer::control()
{
    // use these from status
    //setDirection(v);
    //setRotation(v);
}

void RemotePlayer::control(const ControlPacket* packet)
{
    // update status from packet
}
