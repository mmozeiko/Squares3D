#ifndef __WORLD_H__
#define __WORLD_H__

#include <Newton.h>
#include "common.h"
#include "renderable.h"

class Camera;
class Player;
class Ball;

namespace LevelObjects
{
    class Level;
}

class World : public Renderable
{
public:
	World(Game* game);
    ~World();

    void init();
    
    void control(const Input* input);
    void update(float delta);
    void prepare();
    void render(const Video* video) const;

	auto_ptr<Camera>                m_camera;
    NewtonWorld*                    m_world;
    
private:
    auto_ptr<Player>                m_localPlayer;
    auto_ptr<Ball>                  m_ball;
    auto_ptr<LevelObjects::Level>   m_level;

};

#endif
