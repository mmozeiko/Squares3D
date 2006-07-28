#include "player.h"
#include "camera.h"
#include "world.h"
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

/*
struct ContactBodies
{
    const NewtonBody* body1;
    const NewtonBody* body2;
};

// this callback is called when the aabb box with player overlaps floor
int  PlayerContactBegin(const NewtonMaterial* material, const NewtonBody* body1, const NewtonBody* body2)
{
	ContactBodies* contactBodies = static_cast<ContactBodies*>(NewtonMaterialGetMaterialPairUserData(material));
    contactBodies->body1 = body1;
    contactBodies->body2 = body2;

    return 1;
}

static int PlayerContactProcess(const NewtonMaterial* material, const NewtonContact* contact)
{
	ContactBodies* contactBodies = static_cast<ContactBodies*>(NewtonMaterialGetMaterialPairUserData(material));
    
	Player* player[2];

    player[0] = static_cast<Player*>(NewtonBodyGetUserData(contactBodies->body2));
    player[1] = static_cast<Player*>(NewtonBodyGetUserData(contactBodies->body1));
    
    for (int i=0; i<2; i++)
    {
        if (player[i] && player[i]->getType()==Body::PlayerBody)
        {
            player[i]->onCollision(material, contact);
        }
    }

    return 1;
}

ContactBodies contactBodies;
*/

Music* music;
/*
unsigned int _iCubeTextureID;

void loadCubemap(const string& filename)
{
    
void load_png_cubemap(const char * string, bool mipmap)
{
	char buff[1024];
	GLenum tgt = array_texture_target;

	array2<vec3ub> cubeface;
	
	sprintf(buff, string, "posx");
	read_png_rgb(buff, cubeface);
	array_texture_target = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;
	make_rgb_texture(cubeface, mipmap);

	sprintf(buff, string, "negx");
	read_png_rgb(buff, cubeface);
	array_texture_target = GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB;
	make_rgb_texture(cubeface, mipmap);

	sprintf(buff, string, "posy");
	read_png_rgb(buff, cubeface);
	array_texture_target = GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB;
	make_rgb_texture(cubeface, mipmap);

	sprintf(buff, string, "negy");
	read_png_rgb(buff, cubeface);
	array_texture_target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB;
	make_rgb_texture(cubeface, mipmap);

	sprintf(buff, string, "posz");
	read_png_rgb(buff, cubeface);
	array_texture_target = GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB;
	make_rgb_texture(cubeface, mipmap);

	sprintf(buff, string, "negz");
	read_png_rgb(buff, cubeface);
	array_texture_target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB;
	make_rgb_texture(cubeface, mipmap);

    GL_TEXTURE_CUBE_MAP_ARB


	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
*/

World::World(Game* game) : 
    Renderable(game),
    m_camera(new Camera(game))
{
    m_world = NewtonCreate(NULL, NULL);
    NewtonWorldSetUserData(m_world, static_cast<void*>(this));
    NewtonSetSolverModel(m_world, 10);
    NewtonSetFrictionModel(m_world, 1);
    
    //music = new Sound("music.ogg");
    music = m_game->m_audio->loadMusic("music.ogg");
    music->play();
/*
    glGenTextures(1, &_iCubeTextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _iCubeTextureID);

    loadCubemap("cube_face_");

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    */
}

void World::init()
{
	int charID = NewtonMaterialCreateGroupID(m_world);

	//NewtonMaterialSetDefaultElasticity(m_world, floorID, charID, 0.4f);
	//NewtonMaterialSetDefaultFriction(m_world, floorID, charID, 0.4f, 0.4f);

	//NewtonMaterialSetCollisionCallback(m_world, floorID, charID, 
 //       static_cast<void*>(&contactBodies), 
 //       PlayerContactBegin, PlayerContactProcess, NULL); 

 //   NewtonBodySetMaterialGroupID(floorBody, floorID);

    m_level.reset(new LevelObjects::Level(m_game));
    m_level->load("/data/level.xml");

    m_localPlayers.push_back(new LocalPlayer("playerDura", m_game, Vector(1.0f, 2.0f, 0.0f), Vector(0.0f, 0.0f, 0.0f)));
    m_localPlayers.push_back(new LocalPlayer("player", m_game, Vector(4.0f, 2.0f, 2.0f), Vector(180.0f, 0.0f, 0.0f)));
    m_localPlayers.push_back(new AiPlayer("penguin", m_game, Vector(0.0f, 2.0f, 0.0f), Vector(0.0f, 0.0f, 0.0f)));
}

World::~World()
{
    music->stop();
    m_game->m_audio->unloadMusic(music);

    for each_const(vector<Player*>, m_localPlayers, player)
    {
        delete *player;
    }

    delete m_level.release();

    NewtonMaterialDestroyAllGroupID(m_world);
    NewtonDestroyAllBodies(m_world);
    NewtonDestroy(m_world);
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
    NewtonUpdate(m_world, delta);
}

void World::prepare()
{
    m_camera->prepare();
    m_level->prepare();
}

void World::render(const Video* video) const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_camera->render(video);

    m_level->render(video);

    video->renderAxes();
    glfwSwapBuffers();
}
