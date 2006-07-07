#ifndef __GAME_H__
#define __GAME_H__

#include "config.h"
#include "video.h"
#include "audio.h"
#include "network.h"
#include "player.h"
#include "camera.h"
#include "ball.h"

#include <Newton.h>

class Game
{
public:
    Game();
    ~Game();

    void Run();

private:
    Config  _config;
    Video   _video;
    Audio   _audio;
    Network _network;
    Camera  _camera;

    NewtonWorld* _world;

    Player* _localPlayer;   // temporary
    Ball*   _ball;          // temporary

    void Control(float delta);
    void Update();
    void Prepare();
    void Render() const;
};

#endif
