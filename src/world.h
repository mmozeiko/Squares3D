#ifndef __WORLD_H__
#define __WORLD_H__

#include <Newton.h>
#include "common.h"
#include "renderable.h"

class Camera;
class Player;

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
    auto_ptr<LevelObjects::Level>   m_level;
    vector<Player*>                 m_localPlayers;

};

#endif
