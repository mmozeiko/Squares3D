#include <sstream>
#include "world.h"
#include "player.h"
#include "camera.h"
#include "video.h"
#include "audio.h"
#include "game.h"
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

World::World(Game* game) : Renderable(game)
{
    m_camera = new Camera(m_game, Vector(0.0f, 25.0f, 0.0f), 90.0f, 0.0f);
    m_skybox = new SkyBox(m_game->m_video);

    m_newtonWorld = NewtonCreate(NULL, NULL);
    NewtonWorldSetUserData(m_newtonWorld, static_cast<void*>(this));
    NewtonSetSolverModel(m_newtonWorld, 10);
    NewtonSetFrictionModel(m_newtonWorld, 1);
    
    m_music = m_game->m_audio->loadMusic("music.ogg");
    //m_music->play();
}

void World::init()
{
    m_level = new Level(m_game);
    m_level->load("level.xml");
	m_referee = new Referee();

    // TODO: move to level file
    int id = NewtonMaterialGetDefaultGroupID(m_newtonWorld);
    NewtonMaterialSetDefaultFriction(m_newtonWorld, id, id, 0.0, 1.0);
    NewtonMaterialSetDefaultFriction(m_newtonWorld, id, m_level->m_properties->getPropertyID("penguin"), 0.0f, 0.0f);

    NewtonBodySetContinuousCollisionMode(m_level->getBody("football")->m_newtonBody, 1);

    m_ball = new Ball(m_level->getBody("football"), m_game);
	m_referee->m_ball = m_ball->m_body;
    m_ball->m_referee = m_referee;
	m_referee->m_ground = m_level->getBody("level");

    m_localPlayers.push_back(new LocalPlayer("player", m_game, Vector(-1.5f, 2.0f, -1.5f), Vector(0.0f, 0.0f, 0.0f)));
	m_localPlayers.back()->m_referee = m_referee;
	m_referee->registerPlayer("player1", m_localPlayers.back());

    int i = 0;
    //todo: position ai players without such hacks
	for (float x = 1.5f; x >= -1.5f; x -= 3.0f)
    { 
        for (float z = -1.5f; z <= 1.5f; z += 3.0f)
        { 
			if ((x != -1.5f) || (z != -1.5f))
			{
				Vector pos(x, 1.0f, z);
				m_localPlayers.push_back(new AiPlayer("penguin" + cast<string>(i), m_game, pos, Vector(0.0f, 0.0f, 0.0f)));
				m_localPlayers.back()->m_referee = m_referee;
				stringstream ss;
				ss << i;
				m_referee->registerPlayer("ai_player" + ss.str(), m_localPlayers.back());
				i++;
			}
        }
    }
	m_referee->m_playersP = &m_localPlayers;
}

World::~World()
{
    m_music->stop();
    m_game->m_audio->unloadMusic(m_music);

    for each_const(vector<Player*>, m_localPlayers, player)
    {
        delete *player;
    }

    delete m_level;
	delete m_referee;
    delete m_ball;

    NewtonMaterialDestroyAllGroupID(m_newtonWorld);
    NewtonDestroyAllBodies(m_newtonWorld);
    NewtonDestroy(m_newtonWorld);

    delete m_skybox;
    delete m_camera;
}

void World::control(const Input* input)
{
    if (glfwGetWindowParam(GLFW_ACTIVE) == GL_TRUE)
    {
        m_camera->control(input);
        for each_const(vector<Player*>, m_localPlayers, player)
        {
            (*player)->control(input);
        }
    }

    // other objects go here
    // ...
}

void World::update(float delta)
{
    NewtonUpdate(m_newtonWorld, delta);
}

void World::prepare()
{
    m_camera->prepare();
    m_level->prepare();
}

void World::render(const Video* video) const
{
    glClear(GL_DEPTH_BUFFER_BIT);

    m_camera->render(video);

    m_skybox->render(video); // !! immediately  after camera render !!

    m_level->render(video);

    video->renderAxes();
}
