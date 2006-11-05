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
    m_tmpProfile(NULL),
    m_ready_count(0),
    m_needToStartGame(false),
    m_needToBeginGame(false),
    m_needToQuitGame(false)
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
    m_needToStartGame = false;
    m_needToBeginGame = false;
    m_needToQuitGame = false;
    m_playing = false;
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
    m_needToStartGame = false;
    m_needToBeginGame = false;
    m_needToQuitGame = false;
    m_playing = false;
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
    m_needToStartGame = false;
    m_needToBeginGame = false;
    m_needToQuitGame = false;
    m_playing = false;
    m_activeBodies.clear();

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
    if (m_host == NULL) // no network, single player game!
    {
        return;
    }

    if (m_playing && !m_needToQuitGame)
    {
        if (m_timer.read() > 0.1f) // 100ms (10fps)
        {
            // update local player to remote players
            ControlPacket* packet = m_players[m_localIdx]->getControl();
            packet->m_idx = m_localIdx;
            if (m_isServer)
            {
                for each_(PlayerMap, m_clients, client)
                {
                    send(client->first, *packet, false);
                }

                for (size_t i=0; i<m_activeBodies.size(); i++)
                {
                    UpdatePacket p(static_cast<byte>(i), m_activeBodies[i]);
                    for each_(PlayerMap, m_clients, client)
                    {
                        send(client->first, p, false);
                    }
                }
            }
            else // client
            {
                send(m_server, *packet, false);

                for (size_t i=0; i<m_activeBodies.size(); i++)
                {
                    if (m_players[m_localIdx]->m_body == m_activeBodies[i])
                    {
                        UpdatePacket p(static_cast<byte>(i), m_players[m_localIdx]->m_body);
                        send(m_server, p, false);
                        break;
                    }
                }
            }
            delete packet;
            
            m_timer.reset();
        }
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
                if (m_inMenu == false)
                {
                    enet_peer_disconnect(event.peer);
                    enet_host_flush(m_host);
                }
                else
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
                if (foundIn(m_clients, event.peer))
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
                m_needToQuitGame = true;
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
    }
}

void Network::add(Body* body)
{
    m_activeBodies.push_back(body);
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
        temp.insert(temp.end(), profiles[3].begin(), profiles[3].end());
        
        m_allProfiles[0] = profiles[0];
        m_allProfiles[1] = profiles[1];
        m_allProfiles[2] = profiles[2];
        m_allProfiles[3] = profiles[3];
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
        i = Randoms::getIntN(3);
        k = Randoms::getIntN(static_cast<int>(m_allProfiles[i].size()));
        found = foundIn(m_profiles, m_allProfiles[i][k]);
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
            if (foundIn(m_clients, peer))
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
            
            if (!m_inMenu)
            {
                m_needToQuitGame = true;
            }
        }
        else if (type == Packet::ID_PLACE)
        {
            // not possible
            clog << "WARNING: " << Exception("invalid server packet type = ID_PLACE") << endl;
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

            m_ready_count++;

            for each_(PlayerMap, m_clients, client)
            {
                if (client->first != peer)
                {
                    send(client->first, ReadyPacket(), true);
                }
            }

            if (m_ready_count == 4)
            {
                m_needToBeginGame = true;
                m_playing = true;
            }
        }
        else if (type == Packet::ID_UPDATE)
        {
            // only for remote client
            UpdatePacket p(packet);
            m_activeBodies[p.m_idx]->update(p);
        }
        else if (type == Packet::ID_CONTROL)
        {
            // recieve control from remote player, forward it to other remote players
            ControlPacket p(packet);
            
            for each_(PlayerMap, m_clients, client)
            {
                if (client->first != peer)
                {
                    send(client->first, p, false);
                }
            }
            
            // control remote player on server
            m_players[p.m_idx]->control(p);
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
            clog << "Profile: name=" << prof->m_name << ", cid=" << prof->m_collisionID << endl;
            m_garbage.insert(prof);
        }
        else if (type == Packet::ID_QUIT)
        {
            // server is quitting
            if (m_inMenu)
            {
                m_menu->setSubmenu(m_joinSubmenu);
            }
            m_needToQuitGame = true;
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

            StartPacket p(packet);
            m_ready_count = p.m_ai_count;
            m_needToStartGame = true;
        }
        else if (type == Packet::ID_READY)
        {
            // recieve from server, that some client is ready
            // when all remote clients is ready, start the game
            
            m_ready_count++;
            
            if (m_ready_count == 4)
            {
                m_needToBeginGame = true;
                m_playing = true;
            }
        }
        else if (type == Packet::ID_UPDATE)
        {
            // recieve update from server, update body
            UpdatePacket p(packet);
            
            // ignore update, if server tries to update this client player body
            if (m_players[m_localIdx]->m_body != m_activeBodies[p.m_idx])
            {
                m_activeBodies[p.m_idx]->update(p);
            }
        }
        else if (type == Packet::ID_CONTROL)
        {
            // recieve control from remote player (on server, or other client), update player
            ControlPacket p(packet);
            m_players[p.m_idx]->control(p);
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
    
    m_ready_count = ai_count;
    m_needToStartGame = true;
}

void Network::iAmReady()
{
    if (m_isSingle == false && m_isServer == false)
    {
        m_ready_count++;

        send(m_server, ReadyPacket(), true);
        
        if (m_ready_count==4)
        {
            m_needToBeginGame = true;
            m_playing = true;
        }
    }
    else if (m_isServer == true)
    {
        m_ready_count++;

        for each_(PlayerMap, m_clients, client)
        {
            send(client->first, ReadyPacket(), true);
        }

        if (m_ready_count==4)
        {
            m_needToBeginGame = true;
            m_playing = true;
        }
    }
}
