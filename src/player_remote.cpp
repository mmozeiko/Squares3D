#include "player_remote.h"
#include "world.h"
#include "body.h"
#include "random.h"
#include "geometry.h"
#include "packet.h"
#include "xml.h"

RemotePlayer::RemotePlayer(const string& id, const Vector& position, const Vector& rotation) :
    Player(XMLnode(), NULL) //id, position, rotation)
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
