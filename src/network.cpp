#include "network.h"
#include "random.h"
#include "body.h"

const int SQUARES_PORT = 12321;

template <class Network> Network* System<Network>::instance = NULL;

static const ENetCallbacks callbacks = { 
    NULL, NULL,                                                     // malloc, free
    reinterpret_cast<int (ENET_CALLBACK*)(void)>(Random::getInt)    // rand
};

Network::Network() :
    m_needDisconnect(false),
    m_disconnected(false),
    m_isServer(false),
    m_host(NULL),
    m_server(NULL)
{
    clog << "Initializing network." << endl;

    if (enet_initialize_with_callbacks(ENET_VERSION, &callbacks) != 0)
    {
        throw Exception("enet_initialize failed");
    }
}

Network::~Network()
{
    clog << "Closing network." << endl;
    close();
    enet_deinitialize();

    for each_const(ActiveBodySet, m_activeBodies, iter)
    {
        delete *iter;
    }
}

void Network::createServer()
{
    ENetAddress address;

    address.host = ENET_HOST_ANY;
    address.port = SQUARES_PORT;

    m_host = enet_host_create(&address, 3, 0, 0); // 3 clients
    if (m_host == NULL)
    {
        throw Exception("enet_host_create failed");
    }

    m_isServer = true;
}

void Network::createClient()
{
    m_host = enet_host_create(NULL, 1, 0, 0);
    if (m_host == NULL)
    {
        throw Exception("enet_host_create failed");
    }

    m_isServer = false;
    m_disconnected = false;
}

void Network::connect(const string& host)
{
    ENetAddress address;

    if (enet_address_set_host(&address, host.c_str()) != 0)
    {
        throw Exception("enet_address_set_host failed, host unknown)");
    }
    address.port = SQUARES_PORT;

    m_server = enet_host_connect(m_host, &address, 0);
}

void Network::sendDisconnect()
{
    enet_peer_disconnect(m_server, 0);
}

void Network::close()
{
    if (m_server != NULL)
    {
        enet_peer_reset(m_server);
        m_server = NULL;
    }
    if (m_host != NULL)
    {
        enet_host_destroy(m_host);
        m_host = NULL;
    }
}

void Network::update()
{
    if (m_host == NULL)
    {
        return;
    }

    ENetEvent event;

    while (enet_host_service(m_host, &event, 0) > 0)
    {
        string type;
        switch (event.type)
        {
        case ENET_EVENT_TYPE_NONE:
            type = "None";
            break;
        case ENET_EVENT_TYPE_CONNECT:
            type = "Connect"; // peer
            if (!m_isServer) // connected to server
            {
                m_needDisconnect = true;
            }
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            type = "Disconnect"; //peer, data
            if (!m_isServer) // server disconnected
            {
                m_needDisconnect = false;
                m_disconnected = true;
            }
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            type = "Recieve"; // peer, channelID, packer (must destroy)
            enet_packet_destroy(event.packet);
            break;
        default:
            assert(false);
        }

        clog << "Network event: " << type << endl;
    }
}

void Network::add(Body* body)
{
    ActiveBody* ab = new ActiveBody();
    ab->body = body;
    m_activeBodies.insert(ab);
}
