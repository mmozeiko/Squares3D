#ifndef __GAME_H__
#define __GAME_H__

#include "common.h"
#include "vmath.h"
#include "state.h"

const float DT = 0.01f;

class Config;
class Language;
class Video;
class Audio;
class Network;
class Input;
class FPS;
class Profile;

const Vector gravityVec(0.0f, -9.81f, 0.0f);

extern bool   g_needsToReload;
extern string g_optionsEntry;

class Game : NoCopy
{
public:
    Game();
    ~Game();
    
    void run();
    void saveScreenshot(const FPS& fps) const;

    // Singletons
    Config*     m_config;
    Language*   m_language;
    Video*      m_video;
    Audio*      m_audio;
    Network*    m_network;
    Input*      m_input;
    //
    
private:
    State*      m_state;
    Profile*    m_userProfile;
    int         m_difficulty;
    int         m_unlockedDifficulty;
    bool        m_fixedTimestep;

    State* switchState(const State::Type newState);
    void saveUserData();
    void loadUserData();
};

#endif
