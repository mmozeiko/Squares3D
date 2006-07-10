#include "player.h"
#include "camera.h"
#include "ball.h"
#include "world.h"
#include "video.h"
#include "game.h"
#include "player_local.h"

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
        if (player[i] && player[i]->GetType()==Body::PlayerBody)
        {
            player[i]->onCollision(material, contact);
        }
    }

    return 1;
}

ContactBodies contactBodies;

World::World(Game* game) : 
    _camera(new Camera()), State(game)
{
    _world = NewtonCreate(NULL, NULL);
    NewtonWorldSetUserData(_world, static_cast<void*>(this));
    NewtonSetSolverModel(_world, 10);
    NewtonSetFrictionModel(_world, 1);

    NewtonCollision* floorCol = NewtonCreateBox(_world, 100.0f, 1.0f, 100.f, NULL);
    NewtonBody* floorBody = NewtonCreateBody(_world, floorCol);
	NewtonReleaseCollision(_world, floorCol);

    Matrix pos = Matrix::translate(Vector(0.0f, -0.5f, 0.0f));
    NewtonBodySetMatrix(floorBody, pos.m);
    
    int floorID = NewtonMaterialCreateGroupID(_world);
	int charID = NewtonMaterialCreateGroupID(_world);

	NewtonMaterialSetDefaultElasticity(_world, floorID, charID, 0.4f);
	NewtonMaterialSetDefaultFriction(_world, floorID, charID, 0.4f, 0.4f);

	NewtonMaterialSetCollisionCallback(_world, floorID, charID, 
        static_cast<void*>(&contactBodies), 
        PlayerContactBegin, PlayerContactProcess, NULL); 

    NewtonBodySetMaterialGroupID(floorBody, floorID);

    _localPlayer = auto_ptr<LocalPlayer>(new LocalPlayer(_world, charID, Vector(0.0f, 2.0f, 0.0f), Vector(0.75f, 2.0f, 0.75f)));
    _ball = auto_ptr<Ball>(new Ball(_world, Vector(1, 0.2f, 1), 0.2f));
}

World::~World()
{
    delete _ball.release();
    delete _localPlayer.release();

    NewtonMaterialDestroyAllGroupID(_world);
    NewtonDestroyAllBodies(_world);
    NewtonDestroy(_world);
}

void World::Control(float delta)
{
    // read input from keyboard

    _camera->Control(delta);
    _localPlayer->Control();
}

void World::Update()
{
    // update world objects, simulate physics

    NewtonUpdate(_world, DT);
}

void World::Prepare()
{
    // prepare for rendering - store all object state in temporary variables

    _camera->Prepare();
    _localPlayer->Prepare();
    _ball->Prepare();

}
#include "video.h"
void World::Render() const
{
    // render teh world, state of objects are in temporary variables

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _camera->Render();

    _game->video()->RenderAxes();

    _localPlayer->Render(_game->video());
    _ball->Render(_game->video());

    glfwSwapBuffers();
}