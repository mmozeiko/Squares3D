#ifndef __WORLD_H__
#define __WORLD_H__

#include <Newton.h>
#include "common.h"
#include "vmath.h"
#include "system.h"
#include "state.h"
#include "video.h"

class Camera;
class Player;
class SkyBox;
class Level;
class Music;
class RefereeLocal;
class RefereeBase;
class Ball;
class Messages;
class Message;
class ScoreBoard;
class FrameBuffer;
class Grass;
class Profile;
class Game;
class HDR;
class Chat;
class Shader;

typedef vector<Profile*> ProfilesVector;

class World : public State, public System<World>
{
public:
    World(Profile* userProfile, int& unlockable, int current);
    ~World();

    void init();
    
    void control();
    void updateStep(float delta);
    void update(float delta);
    void prepare();
    void render() const;
    State::Type progress();

    Camera*          m_camera;
    SkyBox*          m_skybox;
    Grass*           m_grass;
    NewtonWorld*     m_newtonWorld;
    Level*           m_level;
    vector<Player*>  m_localPlayers;
    Ball*            m_ball;
    RefereeBase*     m_referee;
    Messages*        m_messages;
    ScoreBoard*      m_scoreBoard;

    int            m_current;

private:
    int&           m_unlockable;

    bool           m_freeze;
    Profile*       m_userProfile;
    Message*       m_escMessage;

    void renderScene() const;

    // all below is shadow stuff
    Vector           m_lightPosition;
    Matrix           m_lightProjection;
    Matrix           m_lightView;
    Matrix           m_textureMatrix;

    unsigned int     m_shadowTex;
    unsigned int     m_shadowSize;
    FrameBuffer*     m_framebuffer;
    HDR*             m_hdr;

    void setLight(const Vector& position);

    void setupShadowStuff();
    void killShadowStuff();

    void shadowMapPass1() const;
    void shadowMapPass2() const;
    void shadowMapPass3() const;

    Message* m_waitMessage;
    Chat*    m_chat;
    bool     m_networkPaused;

    Shader*  m_shadowShader;
};

#endif

