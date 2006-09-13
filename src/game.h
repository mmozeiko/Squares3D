#ifndef __GAME_H__
#define __GAME_H__

#include "common.h"
#include "vmath.h"
#include "state.h"

#define DT 0.01f

class Config;
class Video;
class Audio;
class Network;
class Input;
class World;
class FPS;
class Menu;

const Vector gravityVec(0.0f, -9.81f, 0.0f);

class Game : NoCopy
{
public:
    Game();
    ~Game();
    
    void run();
    void saveScreenshot(const FPS& fps) const;
    State* switchState(const State::Type newState) const;

    Config*  m_config;
    Video*   m_video;
    Audio*   m_audio;
    Network* m_network;
    Input*   m_input;
    World*   m_world;
    Menu*    m_menu;
    State*   m_state;

};

#endif
