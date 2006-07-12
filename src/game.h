#ifndef __GAME_H__
#define __GAME_H__

#include "common.h"

#define DT 0.01f

class Config;
class Video;
class Audio;
class Network;
class Input;
class World;

class Game
{
public:
    Game();
    ~Game();
	
    void run();

    auto_ptr<Config>    m_config;
    auto_ptr<Video>     m_video;
    auto_ptr<Audio>     m_audio;
    auto_ptr<Network>   m_network;
    auto_ptr<Input>     m_input;
    auto_ptr<World>     m_world;
};

#endif
