#include <GL/glfw.h>

#include "network.h"
#include "random.h"
#include "body.h"
#include "profile.h"
#include "player_ai.h"
#include "player_local.h"
#include "player_remote.h"
#include "network.h"
#include "packet.h"
#include "menu.h"
#include "game.h"

const int SQUARES_PORT = 12321;

template <class Network> Network* System<Network>::instance = NULL;

Network::Network() :
    m_needDisconnect(false),
    m_disconnected(false),
    m_isSingle(true),
    m_inMenu(false),
    m_isServer(false),
    m_host(NULL),
    m_server(NULL),
    m_menu(NULL),
    m_tmpProfile(NULL)
{
    clog << "Initializing network." << endl;

    if (enet_initialize() != 0)
    {
        throw Exception("enet_initialize failed");
    }
    
    m_profiles.resize(4);

    m_tmpProfile = new Profile();
    m_tmpProfile->m_name = "???";
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
    delete m_tmpProfile;
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

    m_clients.clear();
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

    return true;
}

void Network::close()
{
    if (!m_isServer && m_server != NULL)
    {
        // TODO: make asynchronous
        send(m_server, QuitPacket(), true);
        enet_host_flush(m_host);
        enet_peer_disconnect(m_server);
        enet_host_flush(m_host);
    }
    if (m_isServer && m_host != NULL)
    {
        // TODO: make asynchronous
        for each_(PlayerMap, m_clients, client)
        {
            send(client->first, QuitPacket(), true);
            enet_host_flush(m_host);
            enet_peer_disconnect(client->first);
            enet_host_flush(m_host);
        }
    }
    
    for each_const(set<Profile*>, m_garbage, iter)
    {
        delete *iter;
    }
    m_garbage.clear();

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
            //type = "None"; wtf?
            break;

        case ENET_EVENT_TYPE_CONNECT:
            type = "Connect"; // peer
            if (m_isServer)
            {
                int freePlace = -1;
                for (int i=0; i<4; i++)
                {
                    if (m_aiIdx[i] && freePlace == -1)
                    {
                        freePlace = i;
                        m_aiIdx[freePlace] = false;
                    }
                    
                    if (i != freePlace)
                    {
                        send(event.peer, JoinPacket(i, m_profiles[i]), true);
                    }
                }

                if (freePlace == -1)
                {
                    send(event.peer, KickPacket("No free places!!"), true);
                }
                else
                {
                    send(event.peer, SetPlacePacket(freePlace), true);
                    m_clients[event.peer] = freePlace;
                }
            }
            else
            {
                m_server = event.peer;
                // connected to server
                m_needDisconnect = true;

                // click the menu button!
                m_menu->setSubmenu(m_lobbySubmenu);
            }
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            type = "Disconnect"; //peer, data
            if (m_isServer)
            {
                if (foundInMap(m_clients, event.peer))
                {
                    // client is quitting
                    int idx = m_clients[event.peer];
                    m_clients.erase(event.peer);
                    m_aiIdx[idx] = true;
                    m_profiles[idx] = getRandomAI();
                
                    for each_(PlayerMap, m_clients, client)
                    {
                        send(client->first, JoinPacket(idx, m_profiles[idx]), true);
                    }
                }
            }
            else
            {
                // server disconnected
                m_needDisconnect = false;
                m_disconnected = true;
                m_server = NULL;
                m_aiIdx[0] = m_aiIdx[1] = m_aiIdx[2] = m_aiIdx[3] = false;
                m_profiles[0] = m_profiles[1] = m_profiles[2] = m_profiles[3] = m_tmpProfile;
                if (m_inMenu)
                {
                    m_menu->setSubmenu(m_joinSubmenu);
                }
            }
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            type = "Recieve"; // peer, channelID, packer (must destroy)
            processPacket(event.peer, bytes(event.packet->data, event.packet->data + event.packet->dataLength));
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
    m_aiIdx[0] = false;
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
    for (int i = 0; i < 3; i++)
    {
        m_profiles[1+i] = temp[i];
        m_aiIdx[1+i] = true;
    }
}

Profile* Network::getRandomAI()
{
    bool found = true;
    int i, k;
    while (found)
    {
        i = Random::getIntN(3);
        k = Random::getIntN(static_cast<int>(m_allProfiles[i].size()));
        found = foundInVector(m_profiles, m_allProfiles[i][k]);
    }
    return m_allProfiles[i][k];
}

void Network::createRemoteProfiles()
{
    for (int i = 0; i < 4; i++)
    {
        m_profiles[i] = m_tmpProfile;
        m_aiIdx[i] = false;
    }
}

const vector<Player*>& Network::createPlayers(Level* level)
{
    m_players.resize(4);
    for (int i=0; i<4; i++)
    {
        if (i == m_localIdx)
        {
            m_players[i] = new LocalPlayer(m_profiles[i], level);
        }
        else if (m_aiIdx[i])
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
            if (m_profiles[k] == m_allProfiles[i][found])
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
    return idx == m_localIdx || m_aiIdx[idx];
}

void Network::send(ENetPeer* peer, const Packet& packet, bool important)
{
    ENetPacket* p = enet_packet_create(&packet.data()[0], packet.data().size(), (important ? ENET_PACKET_FLAG_RELIABLE : 0));
    enet_peer_send(peer, 0, p);
}

void Network::setMenuEntries(Menu* menu, const string& lobbySubmenu, const string& joinSubmenu)
{
    m_menu = menu;
    m_lobbySubmenu = lobbySubmenu;
    m_joinSubmenu = joinSubmenu;
}

void Network::processPacket(ENetPeer* peer, const bytes& packet)
{
    if (packet.size() < 1)
    {
        clog << "WARNING: " << Exception("Invalid packet size == 0") << endl;
        return;
    }

    if (m_isServer)
    {

        const byte type = packet[0];
        
        if (type == Packet::ID_JOIN)
        {
            // client is telling its profile
            JoinPacket p(packet);
            for (int i=0; i<4; i++)
            {
                if (p.m_profile->m_name == m_profiles[i]->m_name)
                {
                    send(peer, KickPacket("player name already in use"), true);
                    return;
                }
            }
            m_aiIdx[p.m_idx] = false;
            Profile* prof = new Profile(*p.m_profile);
            m_profiles[p.m_idx] = prof;
            m_garbage.insert(prof);

            for each_(PlayerMap, m_clients, client)
            {
                if (client->first != peer)
                {
                    send(client->first, JoinPacket(p.m_idx, prof), true);
                }
            }
        }
        else if (type == Packet::ID_QUIT)
        {
            // client is quitting
            if (foundInMap(m_clients, peer))
            {
                int idx = m_clients[peer];
                m_clients.erase(peer);
                m_aiIdx[idx] = true;
                m_profiles[idx] = getRandomAI();

                for each_(PlayerMap, m_clients, client)
                {
                    send(client->first, JoinPacket(idx, m_profiles[idx]), true);
                }
            }

        }
        else if (type == Packet::ID_PLACE)
        {
            // not possible
            clog << "WARNING: " << Exception("invalid server packet type = :ID_PLACE") << endl;
        }
        else if (type == Packet::ID_KICK)
        {
            // not possible
            clog << "WARNING: " << Exception("invalid server packet type = ID_KICK") << endl;
        }
        else if (type == Packet::ID_CHAT)
        {
            // not used
        }
        else if (type == Packet::ID_START)
        {
            // not possible
            clog << "WARNING: " << Exception("invalid server packet type = ID_START") << endl;
        }
        else if (type == Packet::ID_READY)
        {
            // recieve from client when it has loaded game, forward it to other clients
            // when all remote clients is ready, send start to all clients
            // ...
        }
        else if (type == Packet::ID_UPDATE)
        {
            // not possible
            clog << "WARNING: " << Exception("invalid server packet type = ID_UPDATE") << endl;
        }
        else if (type == Packet::ID_CONTROL)
        {
            // recieve control from remote player, forward it to other remote players
            // ...
        }
        else
        {
            clog << "WARNING: " << Exception("invalid packet type = ") << type << endl;
        }

    }
    else
    {

        const byte type = packet[0];
        
        if (type == Packet::ID_JOIN)
        {
            // server is telling who is connected
            JoinPacket p(packet);
            m_aiIdx[p.m_idx] = false;
            Profile* prof = new Profile(*p.m_profile);
            m_profiles[p.m_idx] = prof;
            m_garbage.insert(prof);
        }
        else if (type == Packet::ID_QUIT)
        {
            // server is quitting
            if (m_inMenu)
            {
                m_menu->setSubmenu(m_joinSubmenu);
            }
        }
        else if (type == Packet::ID_PLACE)
        {
            SetPlacePacket p(packet);
            m_localIdx = p.m_idx;
            m_profiles[m_localIdx] = Game::instance->m_userProfile;
            send(m_server, JoinPacket(m_localIdx, m_profiles[m_localIdx]), true);
        }
        else if (type == Packet::ID_KICK)
        {
            // kick teh user
            send(m_server, QuitPacket(), true);
            if (m_inMenu)
            {
                m_menu->setSubmenu(m_joinSubmenu);
            }
        }
        else if (type == Packet::ID_CHAT)
        {
            // not used
        }
        else if (type == Packet::ID_START)
        {
            // when recieved from server, switch state to world
            // when finished loading world, send ready to server
            // ...
        }
        else if (type == Packet::ID_READY)
        {
            // recieve from server, that some client is ready
            // when all remote clients is ready, start the game
            // ...
        }
        else if (type == Packet::ID_UPDATE)
        {
            // recieve update from server, update body
        }
        else if (type == Packet::ID_CONTROL)
        {
            // recieve control from remote player, update player
        }
        else
        {
            clog << "WARNING: " << Exception("invalid packet type = ") << type << endl;
        }
    }
}

void Network::setAiProfile(int idx, Profile* ai)
{
    m_profiles[idx] = ai;
}

void Network::updateAiProfile(int idx)
{
    for each_(PlayerMap, m_clients, client)
    {
        send(client->first, JoinPacket(idx, m_profiles[idx]), true);
    }
}

void Network::kickClient(int idx)
{
    ENetPeer* eraseable;
    for each_(PlayerMap, m_clients, client)
    {
        if (client->second == idx)
        {
            eraseable = client->first;
            send(client->first, KickPacket("server kick"), true);
        }
        else
        {
            send(client->first, JoinPacket(idx, m_profiles[idx]), true);
        }
    }
    m_clients.erase(eraseable); 
    m_aiIdx[idx] = true;
}

void Network::startGame()
{
    int ai_count = 0;
    for (int i=0; i<4; i++)
    {
        ai_count += (m_aiIdx[i] ? 1 : 0);
    }

    for each_(PlayerMap, m_clients, client)
    {
        if (client->second != m_localIdx)
        {
            send(client->first, StartPacket(ai_count), true);
        }
    }
}
