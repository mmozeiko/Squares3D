#include "player.h"
#include "camera.h"
#include "ball.h"
#include "world.h"
#include "video.h"
#include "game.h"
#include "player_local.h"
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
    State(game),
    m_camera(new Camera())
{
    m_world = NewtonCreate(NULL, NULL);
    NewtonWorldSetUserData(m_world, static_cast<void*>(this));
    NewtonSetSolverModel(m_world, 10);
    NewtonSetFrictionModel(m_world, 1);

    NewtonCollision* floorCol = NewtonCreateBox(m_world, 100.0f, 1.0f, 100.f, NULL);
    NewtonBody* floorBody = NewtonCreateBody(m_world, floorCol);
	NewtonReleaseCollision(m_world, floorCol);

    Matrix pos = Matrix::translate(Vector(0.0f, -0.5f, 0.0f));
    NewtonBodySetMatrix(floorBody, pos.m);
    
    int floorID = NewtonMaterialCreateGroupID(m_world);
	int charID = NewtonMaterialCreateGroupID(m_world);

	NewtonMaterialSetDefaultElasticity(m_world, floorID, charID, 0.4f);
	NewtonMaterialSetDefaultFriction(m_world, floorID, charID, 0.4f, 0.4f);

	NewtonMaterialSetCollisionCallback(m_world, floorID, charID, 
        static_cast<void*>(&contactBodies), 
        PlayerContactBegin, PlayerContactProcess, NULL); 

    NewtonBodySetMaterialGroupID(floorBody, floorID);

    m_localPlayer.reset(new LocalPlayer(m_world, charID, Vector(1.0f, 2.0f, 0.0f), Vector(0.75f, 2.0f, 0.75f)));
    m_ball.reset(new Ball(m_world, Vector(1, 0.2f, 1), m_game, 0.2f));
    m_level.reset(new Level());
    //m_level->loadLevelData("/data/level.xml");
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

void World::control(float delta)
{
    // read input from keyboard

    m_camera->control(delta);
    m_localPlayer->control();
}

void World::update()
{
    // update world objects, simulate physics

    NewtonUpdate(m_world, DT);
}

void World::prepare()
{
    // prepare for rendering - store all object state in temporary variables

    m_camera->prepare();
    m_localPlayer->prepare();
    m_ball->prepare();

}

void World::render() const
{
    // render teh world, state of objects are in temporary variables

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_camera->render();

    m_game->video()->renderAxes();

    m_localPlayer->render(m_game->video());
    m_ball->render(m_game->video());

    glfwSwapBuffers();
}