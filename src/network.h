#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "common.h"
#include "system.h"

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

    bool m_needDisconnect;
    bool m_disconnected;

private:
    struct Priv_ENetHost;
    struct Priv_ENetPeer;

    bool m_isServer;

    Priv_ENetHost* m_host;
    Priv_ENetPeer* m_server;

};

#endif
