#ifndef __GAME_H__
#define __GAME_H__

#include "common.h"
#include <Newton.h>

class Config;
class Video;
class Audio;
class Network;
class Camera;
class Player;
class Ball;

class Game
{
public:
    Game();
    ~Game();

    void Run();

private:
    auto_ptr<Config>    _config;
    auto_ptr<Video>     _video;
    auto_ptr<Audio>     _audio;
    auto_ptr<Network>   _network;
    auto_ptr<Camera>    _camera;

    auto_ptr<Player>    _localPlayer;   // temporary
    auto_ptr<Ball>      _ball;          // temporary

    NewtonWorld*        _world;

    void Control(float delta);
    void Update();
    void Prepare();
    void Render() const;
};

#endif
