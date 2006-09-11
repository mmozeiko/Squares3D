#ifndef __WORLD_H__
#define __WORLD_H__

#include <Newton.h>
#include "common.h"
#include "vmath.h"
#include "system.h"

class Camera;
class Player;
class SkyBox;
class Level;
class Music;
class Referee;
class Ball;
class Messages;
class ScoreBoard;
class FrameBuffer;

class World : public System<World>, NoCopy
{
public:
    World();
    ~World();

    void init();
    
    void control();
    void updateStep(float delta);
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
    ScoreBoard*      m_scoreBoard;

private:
    void renderScene() const;

    // all below is shadow stuff
    Vector           m_lightPosition;
    Matrix           m_lightProjection;
    Matrix           m_lightView;
    Matrix           m_textureMatrix;

    unsigned int     m_shadowTex;
    unsigned int     m_shadowSize;
    bool             m_withFBO;
    FrameBuffer*     m_framebuffer;

    void setLight(const Vector& position);

    void setupShadowStuff();
    void killShadowStuff();

    void shadowMapPass1() const;
    void shadowMapPass2() const;
    void shadowMapPass3() const;
};

#endif
