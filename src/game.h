#ifndef __GAME_H__
#define __GAME_H__

#include "common.h"
#include "vmath.h"
#include "state.h"

#define DT 0.01f

class Config;
class Language;
class Video;
class Audio;
class Network;
class Input;
class FPS;

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
    State* switchState(const State::Type newState) const;

    // Singletons
    Config*   m_config;
    Language* m_language;
    Video*    m_video;
    Audio*    m_audio;
    Network*  m_network;
    Input*    m_input;

    // Normal
    State*   m_state;
};

#endif
