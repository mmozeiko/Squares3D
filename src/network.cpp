#include "network.h"
#include "random.h"
#include "body.h"
#include "profile.h"
#include "player_ai.h"
#include "player_local.h"
#include "player_remote.h"

const int SQUARES_PORT = 12321;

template <class Network> Network* System<Network>::instance = NULL;

Network::Network() :
    m_needDisconnect(false),
    m_disconnected(false),
    m_isSingle(true),
    m_isServer(false),
    m_connecting(false),
    m_host(NULL),
    m_server(NULL)
{
    clog << "Initializing network." << endl;

    if (enet_initialize() != 0)
    {
        throw Exception("enet_initialize failed");
    }
    
    m_profiles.resize(4);
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
    m_isSingle = false;
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
    m_isSingle = false;
}

bool Network::connect(const string& host)
{
    ENetAddress address;

    if (enet_address_set_host(&address, host.c_str()) != 0)
    {
        clog << "WARNING: " << Exception("enet_address_set_host failed, host unknown") << endl;
        return false;
    }
    address.port = SQUARES_PORT;

    m_server = enet_host_connect(m_host, &address, 0);
    if (m_server == NULL)
    {
        clog << "WARNING: " << Exception("enet_host_connect failed, got NULL") << endl;
        return false;
    }

    m_connecting = true;
    m_cancelConnection = false;
    return true;
}

void Network::sendDisconnect()
{
    enet_peer_disconnect(m_server);
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
    m_isSingle = true;
    m_connecting = false;
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

const vector<Profile*>& Network::getCurrentProfiles() const
{
    return m_profiles;
}

void Network::setPlayerProfile(Profile* player)
{
    m_profiles[0] = player;
    m_localIdx = 0;
}

void Network::setCpuProfiles(const vector<Profile*> profiles[], int level)
{
    vector<Profile*> temp;
    if (level == -1)
    {
        temp.assign(profiles[0].begin(), profiles[0].end());
        temp.insert(temp.end(), profiles[1].begin(), profiles[1].end());
        temp.insert(temp.end(), profiles[2].begin(), profiles[2].end());
        
        m_allProfiles[0] = profiles[0];
        m_allProfiles[1] = profiles[1];
        m_allProfiles[2] = profiles[2];
    }
    else
    {
        temp.assign(profiles[level].begin(), profiles[level].end());
    }

    std::random_shuffle(temp.begin(), temp.end());
    m_aiIdx.clear();
    for (int i = 0; i < 3; i++)
    {
        m_profiles[1+i] = temp[i];
        m_aiIdx.push_back(1+i);
    }
}

const vector<Player*>& Network::createPlayers(Level* level)
{
    m_players.resize(4);
    m_players[0] = new LocalPlayer(m_profiles[m_localIdx], level);
    for (int i=1; i<4; i++)
    {
        if (foundInVector(m_aiIdx, i))
        {
            m_players[i] = new AiPlayer(m_profiles[i], level);
        }
        else
        {
            m_players[i] = new RemotePlayer(m_profiles[i], level);
        }
    }
    
    return m_players;
}

int Network::getLocalIdx() const
{
    return m_localIdx;
}

void Network::changeCpu(int idx, bool forward)
{
    int found = -1;
    int i;
    for (i=0; i<3; i++)
    {
        for (size_t k=0; k<m_allProfiles[i].size(); k++)
        {
            if (m_allProfiles[i][k] == m_profiles[idx])
            {
                found = static_cast<int>(k);
                break;
            }
        }
        if (found != -1)
        {
            break;
        }
    }

    assert(found != -1);

    // TODO: ugly ugly code
    bool ok = false;
    while (ok == false)
    {
        found += (forward ? +1 : -1);

        if (found >= static_cast<int>(m_allProfiles[i].size()))
        {
            i++;
            found = 0;
            if (i >= 3)
            {
                i = 0;
            }
        }
        else if (found < 0)
        {
            i--;
            if (i < 0)
            {
                i = 2;
            }
            found = static_cast<int>(m_allProfiles[i].size()-1);
        }

        ok = true;
        for (int k=0; k<4; k++)
        {
            if (k != found && m_profiles[k] == m_allProfiles[i][found])
            {
                ok = false;
                break;
            }
        }
        
    }

    m_profiles[idx] = m_allProfiles[i][found];
}

bool Network::isLocal(int idx) const
{
    return idx == m_localIdx || foundInVector(m_aiIdx, idx);
}
