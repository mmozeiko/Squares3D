#include "player.h"
#include "camera.h"
#include "ball.h"
#include "world.h"
#include "video.h"
#include "game.h"
#include "player_local.h"
#include "input.h"
#include "level.h"

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

World::World(Game* game) : 
    Renderable(game),
    m_camera(new Camera(game))
{
    m_world = NewtonCreate(NULL, NULL);
    NewtonWorldSetUserData(m_world, static_cast<void*>(this));
    NewtonSetSolverModel(m_world, 10);
    NewtonSetFrictionModel(m_world, 1);
}

void World::init()
{
    //NewtonCollision* floorCol = NewtonCreateBox(m_world, 100.0f, 1.0f, 100.f, NULL);
    //NewtonBody* floorBody = NewtonCreateBody(m_world, floorCol);
	//NewtonReleaseCollision(m_world, floorCol);

 //   Matrix pos = Matrix::translate(Vector(0.0f, -0.5f, 0.0f));
 //   NewtonBodySetMatrix(floorBody, pos.m);
 //   
 //   int floorID = NewtonMaterialCreateGroupID(m_world);
	int charID = NewtonMaterialCreateGroupID(m_world);

	//NewtonMaterialSetDefaultElasticity(m_world, floorID, charID, 0.4f);
	//NewtonMaterialSetDefaultFriction(m_world, floorID, charID, 0.4f, 0.4f);

	//NewtonMaterialSetCollisionCallback(m_world, floorID, charID, 
 //       static_cast<void*>(&contactBodies), 
 //       PlayerContactBegin, PlayerContactProcess, NULL); 

 //   NewtonBodySetMaterialGroupID(floorBody, floorID);

    m_localPlayer.reset(new LocalPlayer(m_game, charID, Vector(1.0f, 2.0f, 0.0f), Vector(0.75, 2, 0.75)));
    m_ball.reset(new Ball(m_game, Vector(1, 0.2f, 1), 0.2f));

    m_level.reset(new LevelObjects::Level(m_game));
    m_level->load("/data/level.xml");
}

World::~World()
{
    delete m_ball.release();
    delete m_localPlayer.release();
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
        m_localPlayer->control(input);
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
    m_localPlayer->prepare();
    m_ball->prepare();
    m_level->prepare();
}

void World::render(const Video* video) const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_camera->render(video);

    //video->renderAxes();

    m_localPlayer->render(video);
    m_ball->render(video);
    m_level->render(video);

    glfwSwapBuffers();
}
