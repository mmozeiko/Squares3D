#ifndef __NETWORK_H__
#define __NETWORK_H__

// glfw/enet includes conflicting macro definitions in headers
#ifdef APIENTRY
#undef APIENTRY
#endif
#ifdef WINGDIAPI
#undef WINGDIAPI
#endif

#include <enet/enet.h>

#include "common.h"
#include "system.h"

class Body;
class Profile;
class Level;
class Player;

struct ActiveBody
{
    Body* body;
    // also controls
    // ..
};

typedef set<ActiveBody*> ActiveBodySet;

struct Client
{
    ENetPeer* peer;
    // other info
    // ..
};

typedef map<ENetPeer*, Client*> ClientMap;

class Network : public System<Network>, NoCopy
{
public:
    Network();
    ~Network();

    void createServer();
    void createClient();

    bool connect(const string& host);
    void sendDisconnect();
    void close();

    void update();

    void add(Body* body);

    void setPlayerProfile(Profile* player);
    void setCpuProfiles(const vector<Profile*>* profiles, int level);
    const vector<Profile*>& getCurrentProfiles() const;
    const vector<Player*>& createPlayers(Level* level);
    int getLocalIdx() const;
    void changeCpu(int idx, bool forward);
    bool isLocal(int idx) const;

    bool m_needDisconnect;
    bool m_disconnected;

    bool m_isSingle;
    bool m_connecting;
    bool m_cancelConnection;

private:
    bool m_isServer;

    ENetHost* m_host;
    ENetPeer* m_server;

    ActiveBodySet m_activeBodies;
    ClientMap     m_clients;

    vector<Profile*> m_allProfiles[3];
    vector<Profile*> m_profiles;
    int              m_localIdx;
    vector<int>      m_aiIdx;
    vector<Player*>  m_players;
};

#endif
