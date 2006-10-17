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
class Packet;
class Menu;

struct ActiveBody
{
    Body* body;
    // also controls
    // ..
};

typedef set<ActiveBody*> ActiveBodySet;

typedef map<ENetPeer*, int> PlayerMap;

class Network : public System<Network>, NoCopy
{
public:
    Network();
    ~Network();

    void createServer();
    void createClient();

    bool connect(const string& host);
    void close();

    void update();

    void add(Body* body);

    void setPlayerProfile(Profile* player);
    void setCpuProfiles(const vector<Profile*>* profiles, int level);
    void createRemoteProfiles();
    void setAiProfile(int idx, Profile* ai);
    Profile* getRandomAI();

    const vector<Profile*>& getCurrentProfiles() const;
    const vector<Player*>& createPlayers(Level* level);
    int getLocalIdx() const;
    void changeCpu(int idx, bool forward);
    bool isLocal(int idx) const;

    void setMenuEntries(Menu* menu, const string& lobbySubmenu, const string& joinSubmenu);

    void send(ENetPeer* peer, const Packet& packet, bool important);
    void processPacket(ENetPeer* peer, const bytes& packet);

    void updateAiProfile(int idx);
    void kickClient(int idx);

    bool m_needDisconnect;
    bool m_disconnected;

    bool m_isSingle;
    bool m_inMenu;

private:
    bool m_isServer;

    ENetHost* m_host;
    ENetPeer* m_server;

    ActiveBodySet m_activeBodies;
    PlayerMap     m_clients;

    vector<Profile*> m_allProfiles[3];
    vector<Profile*> m_profiles;
    int              m_localIdx;
    bool             m_aiIdx[4];
    vector<Player*>  m_players;

    Menu*            m_menu;
    string           m_lobbySubmenu;
    string           m_joinSubmenu;

    Profile*         m_tmpProfile;
    set<Profile*>    m_garbage;
};

#endif
