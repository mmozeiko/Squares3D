#include <sstream>
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

    m_newtonWorld = NewtonCreate(NULL, NULL);
    NewtonWorldSetUserData(m_newtonWorld, static_cast<void*>(this));
    NewtonSetSolverModel(m_newtonWorld, 10);
    NewtonSetFrictionModel(m_newtonWorld, 1);
    
    m_music = Audio::instance->loadMusic("music.ogg");
    //m_music->play();
}

void World::init()
{
    m_level = new Level();
    m_level->load("level.xml");
    m_referee = new Referee();
    m_messages = new Messages();

    NewtonBodySetContinuousCollisionMode(m_level->getBody("football")->m_newtonBody, 1);

    m_ball = new Ball(m_level->getBody("football"));
    m_referee->m_ball = m_ball->m_body;
    m_ball->m_referee = m_referee;
    m_referee->m_ground = m_level->getBody("level");

    Player* human = new LocalPlayer("player", Vector(-1.5f, 2.0f, -1.5f), Vector::Zero);
    human->m_referee = m_referee;
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
                ai ->m_referee = m_referee;
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

    delete m_level;
    delete m_referee;
    delete m_ball;
    delete m_messages;

    NewtonDestroyAllBodies(m_newtonWorld);
    NewtonDestroy(m_newtonWorld);

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

void World::update(float delta)
{
    m_ball->triggerBegin();
    NewtonUpdate(m_newtonWorld, delta);
    m_ball->triggerEnd();

    m_messages->update(delta);
}

void World::prepare()
{
    m_camera->prepare();
    m_level->prepare();
}

void World::render() const
{
    glClear(GL_DEPTH_BUFFER_BIT);

    m_camera->render();

    m_skybox->render(); // !! immediately  after camera render !!

    m_level->render();

    Video::instance->renderAxes();
    
    m_messages->render();
}
