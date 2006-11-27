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
#include "menu_options.h"
#include "game.h"
#include "version.h"
#include "referee_base.h"
#include "world.h"
#include "properties.h"
#include "config.h"
#include "chat.h"
#include "xml.h"

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
    m_needToQuitGame(false),
    m_netfps(1.0f/Config::instance->m_misc.net_fps),
    m_chat(NULL),
    m_localIdx(0)
{
    clog << "Initializing network." << endl;

    if (enet_initialize() != 0)
    {
        throw Exception("enet_initialize failed");
    }
    
    loadLevelList();

    m_profiles.resize(4);

    m_tmpProfile = new Profile();
    m_tmpProfile->m_name = "???";

    for (int i=0; i<4; i++) m_clientReady[i] = false;
}

Network::~Network()
{
    clog << "Closing network." << endl;

    close();
    enet_deinitialize();

    delete m_tmpProfile;
}

void Network::setReferee(RefereeBase* referee)
{
    m_referee = referee;
}

void Network::createServer()
{
    ENetAddress address;

    address.host = ENET_HOST_ANY;
    address.port = cast<unsigned short>(Config::instance->m_misc.net_port);
    if (address.port == 0)
    {
        address.port = cast<unsigned short>(Config::defaultMisc.net_port);
    }

    m_host = enet_host_create(&address, 3, 0, 0); // 3 clients
    if (m_host == NULL)
    {
        //throw Exception("enet_host_create failed");
        m_menu->setSubmenu("infoNetPortFailed");
        return;
    }

    m_clients.clear();
    m_isServer = true;
    m_isSingle = false;
    m_disconnected = false;
    m_needToStartGame = false;
    m_needToBeginGame = false;
    m_needToQuitGame = false;
    m_playing = false;
    m_ready_count = 0;
    m_localIdx = 0;
}

void Network::createClient()
{
    m_host = enet_host_create(NULL, 1, 0, 0);
    if (m_host == NULL)
    {
        //infoNetPortFailed
        throw Exception("enet_host_create failed");
    }

    m_isServer = false;
    m_isSingle = false;
    m_disconnected = false;
    m_needToStartGame = false;
    m_needToBeginGame = false;
    m_needToQuitGame = false;
    m_playing = false;
    m_localIdx = -1;
}

bool Network::connect(const string& host)
{
    ENetAddress address;
    string connect_host;

    string::size_type pos = host.find(':');
    if (pos == string::npos)
    {
        address.port = cast<unsigned short>(Config::defaultMisc.net_port);
        connect_host = host;
    }
    else
    {
        connect_host = host.substr(0, pos);
        address.port = cast<unsigned short>(host.substr(pos+1));
        if (address.port == 0)
        {
            address.port = cast<unsigned short>(Config::defaultMisc.net_port);
        }
    }

    if (enet_address_set_host(&address, connect_host.c_str()) != 0)
    {
        clog << "WARNING: " << Exception("enet_address_set_host failed, host unknown") << endl;
        return false;
    }

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
        enet_peer_disconnect(m_server, 0);
        enet_host_flush(m_host);
    }
    if (m_isServer && m_host != NULL)
    {
        // TODO: make asynchronous
        for each_(PlayerMap, m_clients, client)
        {
            send(client->first, QuitPacket(), true);
            enet_host_flush(m_host);
            enet_peer_disconnect(client->first, 0);
            enet_host_flush(m_host);
        }
    }
    m_needToStartGame = false;
    m_needToBeginGame = false;
    m_needToQuitGame = false;
    m_playing = false;
    for each_const(ActiveBodyVector, m_activeBodies, it)
    {
        delete *it;
    }
    m_activeBodies.clear();

    for each_const(PacketBuffer, m_packetsBuffer, iter)
    {
        delete *iter;
    }
    m_packetsBuffer.clear();

    for each_const(set<Profile*>, m_garbage, iter)
    {
        delete *iter;
    }
    m_garbage.clear();
    
    for (int i=0; i<4; i++) m_clientReady[i] = false;

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
    m_isServer = false;
    m_ready_count = 0;

    m_chat = NULL;
}

void Network::sendUpdatePacket()
{
    for (size_t i=0; i<m_activeBodies.size(); i++)
    {
        Body* b = m_activeBodies[i]->body;
        if (b->isMovable())
        {
            UpdatePacket p(static_cast<byte>(i), b);
            for each_(PlayerMap, m_clients, client)
            {
                send(client->first, p, false);
            }
        }
    }
}

void Network::update()
{
    if (m_host == NULL) // no network, single player game!
    {
        return;
    }

    if (m_playing && !m_needToQuitGame)
    {
        if (m_timer.read() > m_netfps) // 20ms (50fps)
        {
            // update local player to remote players
            if (m_isServer)
            {
                sendUpdatePacket();

                for each_const(PacketBuffer, m_packetsBuffer, iter)
                {
                    for each_const(PlayerMap, m_clients, client)
                    {
                        send(client->first, **iter, true);
                    }
                    delete *iter;
                }
                m_packetsBuffer.clear();
            }
            else // client
            {
                ControlPacket* packet = m_players[m_localIdx]->getControl(m_localIdx);
                if (packet != NULL)
                {
                    send(m_server, *packet, false);
                }
            }
            
            m_timer.reset();
        }
    }
    else if (!m_playing && m_isServer == false && m_server != NULL)
    {
        send(m_server, ReadyPacket(), true);
    }

    ENetEvent event;

    while (m_host != NULL && enet_host_service(m_host, &event, 0) > 0)
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
                    enet_peer_disconnect(event.peer, 0);
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
                            send(event.peer, JoinPacket(i, g_version, m_profiles[i]), true);
                        }
                    }

                    if (freePlace == -1)
                    {
                        send(event.peer, KickPlacesPacket(), true);
                    }
                    else
                    {
                        send(event.peer, SetPlacePacket(freePlace, m_curLevel), true);
                        m_clients[event.peer] = freePlace;
                    }
                }
            }
            else
            {
                m_server = event.peer;
                // connected to server
                m_needDisconnect = true;

                if (m_inMenu)
                {
                    // click the menu button!
                    m_menu->setSubmenu(m_lobbySubmenu);

                    // activate client chat!
                    m_chat = m_menu->m_chat;
                }
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
                        send(client->first, JoinPacket(idx, g_version, m_profiles[idx]), true);
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
                    m_menu->setSubmenu("infoDisconnect");
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
    ActiveBody* ac = new ActiveBody();
    ac->body = body;
    ac->lastPosition = body->m_matrix;
    m_activeBodies.push_back(ac);
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
        i = Randoms::getIntN(4);
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
    for (i=0; i<4; i++)
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
            if (i >= 4)
            {
                i = 0;
            }
        }
        else if (found < 0)
        {
            i--;
            if (i < 0)
            {
                i = 3;
            }
            found = static_cast<int>(m_allProfiles[i].size()-1);
        }

        ok = true;
        for (int k=0; k<4; k++)
        {
            if (m_profiles[k] == m_allProfiles[i][found] ||
                m_profiles[k]->m_color == m_allProfiles[i][found]->m_color)
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

int Network::getBodyIdx(const Body* body) const
{
    for (size_t i = 0; i < m_activeBodies.size(); i++)
    {
        if (m_activeBodies[i]->body == body)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

Body* Network::getBodyByName(const string& name) const
{
    for each_const(ActiveBodyVector, m_activeBodies, iter)
    {
        if ((*iter)->body->m_id == name)
        {
            return (*iter)->body;
        }
    }
    return NULL;
}

void Network::addResetOwnComboPacket(const Body* body)
{
    if (m_isSingle) return;

    int bodyIdx = getBodyIdx(body);
    if (bodyIdx != -1)
    {
        m_packetsBuffer.push_back(new ComboResetOwnPacket(bodyIdx));
    }
}
void Network::addResetComboPacket()
{
    if (m_isSingle) return;

    m_packetsBuffer.push_back(new ComboResetPacket());
}
void Network::addIncrementComboPacket(const Body* body)
{
    if (m_isSingle) return;
    
    int bodyIdx = getBodyIdx(body);
    if (bodyIdx != -1)
    {
        m_packetsBuffer.push_back(new ComboIncPacket(bodyIdx));
    }
}

void Network::addRefereePacket(int faultID, const Body* body, int points)
{
    if (m_isSingle) return;

    if (body == NULL)
    {
        body = m_activeBodies[0]->body; // a little hack for middle line which has no body
    }

    int bodyIdx = getBodyIdx(body);
    if (bodyIdx != -1)
    {
        m_packetsBuffer.push_back(new RefereePacket(faultID, bodyIdx, points));
    }
}

void Network::addSoundPacket(byte id, const Vector& position)
{
    //clog << "SERVER: sound, " << (int)id << endl;
    m_packetsBuffer.push_back(new SoundPacket(id, position));
}

void Network::addChatPacket(const string& msg)
{
    if (m_isSingle)
    {
        return;
    }
    if (m_isServer)
    {
        if (m_inMenu)
        {
            ChatPacket packet(m_localIdx, msg);

            for each_const(PlayerMap, m_clients, client)
            {
                send(client->first, packet, true);
            }
        }
        else
        {
            m_packetsBuffer.push_back(new ChatPacket(m_localIdx, msg));
        }
    }
    else
    {
        send(m_server, ChatPacket(m_localIdx, msg), true);
    }
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

            //clog << "COnnected: " << p.m_profile->m_name << endl;
            for (int i=0; i<4; i++)
            {
                //clog << "Have: " << m_profiles[i]->m_name << endl;
                if (p.m_profile->m_name == m_profiles[i]->m_name)
                {
                    //clog << "KICKING!!" << endl;
                    send(peer, KickNamesPacket(), true);
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
                    send(client->first, JoinPacket(p.m_idx, g_version, prof), true);
                }
            }
        }
        else if (type == Packet::ID_QUIT)
        {
            // client is quitting
            //clog << "SERVER: Packet::ID_QUIT, client is quitting" << endl;
            if (foundIn(m_clients, peer))
            {
                int idx = m_clients[peer];
                //clog << "erasing old, and joinging some ai, idx=" << idx << endl;
                m_clients.erase(peer);
                m_aiIdx[idx] = true;
                m_profiles[idx] = getRandomAI();

                for each_const(PlayerMap, m_clients, client)
                {
                    send(client->first, JoinPacket(idx, g_version, m_profiles[idx]), true);
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
            if (m_chat == NULL)
            {
                return;
            }
            
            ChatPacket p(packet);
            m_chat->recieve(m_profiles[p.m_player]->m_name, m_profiles[p.m_player]->m_color, p.m_msg);

            for each_(PlayerMap, m_clients, client)
            {
                if (client->first != peer)
                {
                    send(client->first, p, true);
                }
            }

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

            int idx = m_clients[peer];
            if (m_clientReady[idx] == false)
            {
                m_clientReady[idx] = true;

                m_ready_count++;

                //clog << "SERVER, Packet::ID_READY, m_ready_count=" << m_ready_count << endl;

                if (m_ready_count >= 4)
                {
                    m_needToBeginGame = true;
                    m_playing = true;

                    for each_(PlayerMap, m_clients, client)
                    {
                        send(client->first, ReadyPacket(), true);
                    }
                }
            }
        }
        else if (type == Packet::ID_UPDATE)
        {
            if (m_inMenu) return;

            // only for remote client
            UpdatePacket p(packet);
            m_activeBodies[p.m_idx]->body->update(p);
        }
        else if (type == Packet::ID_CONTROL)
        {
            if (m_inMenu) return;

            // recieve control from remote player
            ControlPacket p(packet);

            // control remote player on server
            m_players[p.m_idx]->control(p);
        }
        else
        {
            clog << "WARNING: " << Exception("invalid packet type = ") << (int)type << endl;
        }

    }
    else // client
    {

        const byte type = packet[0];
        
        if (type == Packet::ID_JOIN)
        {
            // server is telling who is connected
            JoinPacket p(packet);

            if (p.m_version != g_version)
            {
                send(m_server, QuitPacket(), true);
                if (m_inMenu)
                {
                    g_neededVersion = p.m_version;
                    m_menu->setSubmenu("infoServerVersion");
                }
            }

            m_aiIdx[p.m_idx] = false;
            Profile* prof = new Profile(*p.m_profile);
            m_profiles[p.m_idx] = prof;
            //clog << "Profile: name=" << prof->m_name << ", cid=" << prof->m_collisionID << endl;
            m_garbage.insert(prof);
        }
        else if (type == Packet::ID_QUIT)
        {
            // server is quitting
            if (m_inMenu)
            {
                m_menu->setSubmenu("infoDisconnect");
            }
            m_needToQuitGame = true;
        }
        else if (type == Packet::ID_PLACE)
        {
            SetPlacePacket p(packet);
            m_curLevel = p.m_level;
            m_localIdx = p.m_idx;
            m_profiles[m_localIdx] = Game::instance->m_userProfile;
            send(m_server, JoinPacket(m_localIdx, g_version, m_profiles[m_localIdx]), true);
        }
        else if (type == Packet::ID_KICK)
        {
            //clog << "CLIENT: Packet::ID_KICK, i have been kicked ;(" << endl;
            // kick teh user
            send(m_server, QuitPacket(), true);
            if (m_inMenu)
            {
                m_menu->setSubmenu("infoKick");
            }
            close();
        }
        else if (type == Packet::ID_KICKNAME)
        {
            // kick teh user
            send(m_server, QuitPacket(), true);
            if (m_inMenu)
            {
                m_menu->setSubmenu("infoNameTaken");
            }
            close();
        }
        else if (type == Packet::ID_KICKPLACES)
        {
            // kick teh user
            send(m_server, QuitPacket(), true);
            if (m_inMenu)
            {
                m_menu->setSubmenu("infoServerFull");
            }
            close();
        }
        else if (type == Packet::ID_CHAT)
        {
            if (m_chat == NULL)
            {
                return;
            }
            ChatPacket p(packet);
            m_chat->recieve(m_profiles[p.m_player]->m_name, m_profiles[p.m_player]->m_color, p.m_msg);
        }
        else if (type == Packet::ID_START)
        {
            // when recieved from server, switch state to world
            // when finished loading world, send ready to server

            StartPacket p(packet);
            m_needToStartGame = true;
            //clog << "CLIENT, Packet::ID_START" << endl;
        }
        else if (type == Packet::ID_READY)
        {
            // recieve from server, that some client is ready
            // when all remote clients is ready, start the game
            
            //clog << "CLIENT, Packet::ID_READY" << endl;
            m_needToBeginGame = true;
            m_playing = true;
        }
        else if (type == Packet::ID_UPDATE)
        {
            if (m_inMenu) return;

            // recieve update from server, update body
            UpdatePacket p(packet);

            ActiveBody* & ab = m_activeBodies[p.m_idx];
            ab->lastPosition = p.m_position;

            ab->body->setMatrix(ab->lastPosition );
        }
        else if (type == Packet::ID_CONTROL)
        {
            // not possible
            clog << "WARNING: " << Exception("invalid client packet type = ID_CONTROL") << endl;
        }
        else if (type == Packet::ID_RESETCOMBO)
        {
            if (m_inMenu) return;

            m_referee->resetCombo();
        }
        else if (type == Packet::ID_RESETOWNCOMBO)
        {
            if (m_inMenu) return;

            ComboResetOwnPacket p(packet);
            Body* player = m_activeBodies[p.m_bodyID]->body;

            m_referee->resetOwnCombo(player);
        }
        else if (type == Packet::ID_INCCOMBO)
        {
            if (m_inMenu) return;

            ComboIncPacket p(packet);
            Body* player = m_activeBodies[p.m_bodyID]->body;

            m_referee->incrementCombo(player, player->getPosition()); //(+1)
        }
        else if (type == Packet::ID_REFEREE)
        {
            if (m_inMenu) return;

            RefereePacket p(packet);
            Body* body = m_activeBodies[p.m_bodyID]->body;

            Body* ball = getBodyByName("football");
            m_referee->scoreBoardCritical(p.m_faultID, body->m_id, p.m_points, ball->getPosition());
        }
        else if (type == Packet::ID_SOUND)
        {
            if (m_inMenu) return;

            SoundPacket p(packet);
            World::instance->m_level->m_properties->play(p.m_id, p.m_position);
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
        send(client->first, JoinPacket(idx, g_version, m_profiles[idx]), true);
    }
}

void Network::kickClient(int idx)
{
    //clog << "Network::kickClient - kicking client idx=" << idx << endl;
    ENetPeer* eraseable = NULL;
    for each_(PlayerMap, m_clients, client)
    {
        if (client->second == idx)
        {
            //eraseable = client->first;
            send(client->first, KickPacket("kick"), true);
        }
        else
        {
            send(client->first, JoinPacket(idx, g_version, m_profiles[idx]), true);
        }
    }
    //m_clients.erase(eraseable); 
    m_aiIdx[idx] = true;
}

void Network::startGame()
{
    int ai_count = 0;
    for (int i=0; i<4; i++)
    {
        ai_count += (m_aiIdx[i] ? 1 : 0);
    }
    
    for each_const(PlayerMap, m_clients, client)
    {
        if (client->second != m_localIdx)
        {
            send(client->first, StartPacket(), true);
        }
    }
    
    //clog << "SERVER, m_ready_count=" << m_ready_count << endl;
    m_ready_count += ai_count;
    m_needToStartGame = true;

    if (m_ready_count >= 4)
    {
        m_needToBeginGame = true;
        m_playing = true;

        for each_const(PlayerMap, m_clients, client)
        {
            send(client->first, ReadyPacket(), true);
        }
    }

}

void Network::iAmReady()
{
    if (m_isSingle == false && m_isServer == false)
    {
        send(m_server, ReadyPacket(), true);
    }
    else if (m_isServer == true)
    {
        m_ready_count++;

        //clog << "SERVER, m_ready_count=" << m_ready_count << endl;
        if (m_ready_count >= 4)
        {
            m_needToBeginGame = true;
            m_playing = true;

            for each_const(PlayerMap, m_clients, client)
            {
                send(client->first, ReadyPacket(), true);
            }
        }
    }
}

void Network::setChat(Chat* chat)
{
    m_chat = chat;
}

string Network::getMaxPlayerName() const
{
    string m;
    for (int i=0; i<4; i++)
    {
        if (m_profiles[i]->m_name.size() > m.size())
        {
            m = m_profiles[i]->m_name;
        }
    }
    return m;
}

Vector Network::getProfileColor(size_t idx) const
{
    assert(idx >=0 && idx < 4);
    return m_profiles[idx]->m_color;
}

string Network::getLevel() const
{
    return m_levelFiles[m_curLevel];
}

void Network::loadLevelList()
{
    XMLnode xml;
    File::Reader in("/data/level/level_list.xml");
    if (!in.is_open())
    {
        throw Exception("Level file '/data/level/level_list.xml' not found");  
    }
    xml.load(in);
    in.close();

    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "level")
        {
            m_levelFiles.push_back(node.getAttribute("file"));
        }
        else
        {
            throw Exception("Invalid cpu_profiles, unknown node - " + node.name);
        }
    }

    m_curLevel = 0;
    m_levelEntry = NULL;
}

void Network::setLevelOption(const OptionEntry* entry)
{
    m_levelEntry = entry;
}

void Network::updateLevelOption()
{
    m_curLevel = m_levelEntry->getCurrentValueIdx();
}

size_t Network::getLevelCount() const
{
    return m_levelFiles.size();
}
