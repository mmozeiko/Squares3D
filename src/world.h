#ifndef __WORLD_H__
#define __WORLD_H__

#include <Newton.h>
#include "common.h"
#include "state.h"

class Camera;
class Player;
class Ball;
class Level;

class World : public State
{
public:
	World(Game* game);
    ~World();
    
    void control(float delta);
    void update();
    void prepare();
    void render() const;
    
private:
	auto_ptr<Camera>    m_camera;
    auto_ptr<Player>    m_localPlayer;
    auto_ptr<Ball>      m_ball;
    auto_ptr<Level>      m_level;

    NewtonWorld*        m_world;
};

#endif
