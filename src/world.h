#ifndef __WORLD_H__
#define __WORLD_H__

#include <Newton.h>
#include "common.h"
#include "state.h"

class Camera;
class Player;
class Ball;

class World : public State
{
public:
	World(Game* game);
    ~World();
    void Control(float delta);
    void Update();
    void Prepare();
    void Render() const;
    
private:
	auto_ptr<Camera>    _camera;
    auto_ptr<Player>    _localPlayer;
    auto_ptr<Ball>      _ball;
    NewtonWorld*        _world;
};
#endif