#ifndef __GAME_H__
#define __GAME_H__

#include "common.h"

#define DT         0.01f

class Config;
class Video;
class Audio;
class Network;
class World;

class Game
{
public:
    Game();
    ~Game();
	const Video* video() const;

    void Run();

private:
    auto_ptr<Config>    _config;
    auto_ptr<Video>     _video;
    auto_ptr<Audio>     _audio;
    auto_ptr<Network>   _network;
    auto_ptr<World>     _world;

};

#endif
