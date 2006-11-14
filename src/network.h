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
#include "timer.h"
#include "system.h"
#include "vmath.h"

class Body;
class Profile;
class Level;
class Player;
class Packet;
class Menu;
class RefereeBase;
class RemotePlayer;
class RefereeProcessPacket;
class SoundPacket;
class Chat;
class OptionEntry;

struct ActiveBody
{
    Body*  body;

    Matrix lastPosition;
};

typedef vector<ActiveBody*> ActiveBodyVector;

typedef map<ENetPeer*, int> PlayerMap;

typedef vector<const Packet*> PacketBuffer;

class Network : public System<Network>, public NoCopy
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
    void setCpuProfiles(const vector<Profile*> profiles[], int level);
    void createRemoteProfiles();
    void setAiProfile(int idx, Profile* ai);
    Profile* getRandomAI();

    const vector<Profile*>& getCurrentProfiles() const;
    const vector<Player*>& createPlayers(Level* level);
    int getLocalIdx() const;
    void changeCpu(int idx, bool forward);
    bool isLocal(int idx) const;

    void setMenuEntries(Menu* menu, const string& lobbySubmenu, const string& joinSubmenu);
    
    void setReferee(RefereeBase* referee);
    void setChat(Chat* chat);

    void updateAiProfile(int idx);
    void kickClient(int idx);
    void startGame(); // server->client, when starting world
    void iAmReady();  // client->server, when world is loaded

    void addSoundPacket(byte id, const Vector& position);
    void addResetOwnComboPacket(const Body* b1);
    void addResetComboPacket();
    void addIncrementComboPacket(const Body* b1);
    void addRefereePacket(int faultID, const Body* b1, int points);
    void addChatPacket(const string& msg);

    Body* getBodyByName(const string& name) const;
    int getBodyIdx(const Body* body) const;

    string getMaxPlayerName() const;
    Vector getProfileColor(size_t idx) const;

    bool m_needDisconnect;
    bool m_disconnected;

    bool m_isServer;
    bool m_isSingle;
    bool m_inMenu;

    bool m_needToStartGame;
    bool m_needToBeginGame;
    bool m_needToQuitGame;
    bool  m_playing;
    Chat* m_chat;

    string getLevel() const;
    string getLevelName(size_t idx) const;
    size_t getLevelCount() const;
    void setLevelOption(const OptionEntry* entry);
    void updateLevelOption();

private:
    ENetHost* m_host;
    ENetPeer* m_server;

    ActiveBodyVector m_activeBodies;
    PlayerMap        m_clients;

    vector<Profile*> m_allProfiles[4];
    vector<Profile*> m_profiles;
    int              m_localIdx;
    bool             m_aiIdx[4];
    vector<Player*>  m_players;

    Menu*            m_menu;
    string           m_lobbySubmenu;
    string           m_joinSubmenu;

    Profile*         m_tmpProfile;
    set<Profile*>    m_garbage;

    int              m_ready_count;
    Timer            m_timer;
    Timer            m_bodyTimer;

    PacketBuffer     m_packetsBuffer;

    RefereeBase*     m_referee;
    bool             m_clientReady[4];

    float            m_netfps;

    void sendUpdatePacket();
    void send(ENetPeer* peer, const Packet& packet, bool important);
    void processPacket(ENetPeer* peer, const bytes& packet);

    const OptionEntry* m_levelEntry;
    StringVector     m_levelFiles;
    StringVector     m_levelNames;
    byte             m_curLevel;
    void             loadLevelList();
};

#endif
