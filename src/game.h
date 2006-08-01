#ifndef __GAME_H__
#define __GAME_H__

#include "common.h"
#include "vmath.h"

#define DT 0.01f

class Config;
class Video;
class Audio;
class Network;
class Input;
class World;

const Vector gravityVec(0.0f, -9.81f, 0.0f);

class Game : NoCopy
{
public:
    Game();
    ~Game();
	
    void run();

    Config*  m_config;
    Video*   m_video;
    Audio*   m_audio;
    Network* m_network;
    Input*   m_input;
    World*   m_world;
};

#endif
