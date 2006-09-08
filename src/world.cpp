#include "world.h"
#include "player.h"
#include "camera.h"
#include "video.h"
#include "audio.h"
#include "player_local.h"
#include "player_ai.h"
#include "input.h"
#include "level.h"
#include "music.h"
#include "sound.h"
#include "file.h"
#include "skybox.h"
#include "properties.h"
#include "referee.h"
#include "ball.h"
#include "messages.h"

World* System<World>::instance = NULL;

World::World()
{
    //m_camera = new Camera(Vector(0.0f, 25.0f, 0.0f), 90.0f, 0.0f);
    m_camera = new Camera(Vector(0.0f, 5.0f, 8.0f), 30.0f, 0.0f);
    m_skybox = new SkyBox();
    m_messages = new Messages();
    m_scoreBoard = new ScoreBoard();

    m_newtonWorld = NewtonCreate(NULL, NULL);

    // enable some Newton optimization
    NewtonSetSolverModel(m_newtonWorld, 10);
    NewtonSetFrictionModel(m_newtonWorld, 1);
    
    m_music = Audio::instance->loadMusic("music.ogg");
    //m_music->play();
}

void World::init()
{
    m_level = new Level();
    m_level->load("level.xml");
    NewtonBodySetContinuousCollisionMode(m_level->getBody("football")->m_newtonBody, 1);

    m_referee = new Referee(m_messages, m_scoreBoard);
    m_referee->m_ground = m_level->getBody("level");

    m_ball = new Ball(m_level->getBody("football"));
    m_referee->registerBall(m_ball);

    Player* human = new LocalPlayer("player", Vector(-1.5f, 2.0f, -1.5f), Vector::Zero);
    m_localPlayers.push_back(human);

    m_referee->registerPlayer("player1", human);
    m_ball->addBodyToFilter(human->m_body);

    int i = 0;
    //todo: position ai players without such hacks
    for (float x = 1.5f; x >= -1.5f; x -= 3.0f)
    { 
        for (float z = -1.5f; z <= 1.5f; z += 3.0f)
        { 
            if ((x != -1.5f) || (z != -1.5f))
            {
                Vector pos(x, 1.0f, z);
                Player* ai = new AiPlayer("penguin" + cast<string>(i), pos, Vector::Zero);
                m_localPlayers.push_back(ai);

                m_referee->registerPlayer("ai_player" + cast<string>(i), m_localPlayers.back());
                m_ball->addBodyToFilter(ai->m_body);
                i++;
            }
        }
    }
}

World::~World()
{
    m_music->stop();
    Audio::instance->unloadMusic(m_music);

    for each_const(vector<Player*>, m_localPlayers, player)
    {
        delete *player;
    }

    delete m_ball;
    delete m_scoreBoard;
    delete m_referee;
    delete m_level;

    NewtonDestroyAllBodies(m_newtonWorld);
    NewtonDestroy(m_newtonWorld);

    delete m_messages;
    delete m_skybox;
    delete m_camera;
}

void World::control()
{
    if (glfwGetWindowParam(GLFW_ACTIVE) == GL_TRUE)
    {
        // only camera and local players
        m_camera->control();
        m_localPlayers.front()->control();
    }
    // other objects go after this

    for (size_t i=1; i<m_localPlayers.size(); i++)
    {
        m_localPlayers[i]->control();
    }
}

void World::updateStep(float delta)
{
    // updateStep is called more than one time in frame

    m_ball->triggerBegin();
    NewtonUpdate(m_newtonWorld, delta);
    m_ball->triggerEnd();
}

void World::update(float delta)
{
    // update is called one time in frame

    m_camera->update(delta);
    m_messages->update(delta);
}

void World::prepare()
{
    m_camera->prepare();
    m_level->prepare();
}

void World::render() const
{
    m_camera->render();
    m_skybox->render(); // !! immediately  after camera render !!

    Video::instance->renderAxes();   

    m_level->render();

    m_messages->render(); // messages is rendered last
}
