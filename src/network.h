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

    void connect(const string& host);
    void sendDisconnect();
    void close();

    void update();

    void add(Body* body);

    bool m_needDisconnect;
    bool m_disconnected;

private:
    bool m_isServer;

    ENetHost* m_host;
    ENetPeer* m_server;

    ActiveBodySet m_activeBodies;
    ClientMap     m_clients;

};

#endif
