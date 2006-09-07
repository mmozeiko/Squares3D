#ifndef __WORLD_H__
#define __WORLD_H__

#include <Newton.h>
#include "common.h"
#include "system.h"

class Camera;
class Player;
class SkyBox;
class Level;
class Music;
class Referee;
class Ball;
class Messages;

class World : public System<World>, NoCopy
{
public:
    World();
    ~World();

    void init();
    
    void control();
    void update(float delta);
    void prepare();
    void render() const;

    Music*           m_music;
    Camera*          m_camera;
    SkyBox*          m_skybox;
    NewtonWorld*     m_newtonWorld;
    Level*           m_level;
    vector<Player*>  m_localPlayers;
    Ball*            m_ball;
    Referee*         m_referee;
    Messages*        m_messages;

};

#endif
