#include "game.h"
#include "timer.h"
#include "player_local.h"

#include "config.h"
#include "video.h"
#include "audio.h"
#include "network.h"
#include "player.h"
#include "camera.h"
#include "ball.h"

#include "vmath.h"

#define DT         0.01f

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

Game::Game() : 
    _config(new Config()), 
    _video(new Video(*_config)), 
    _audio(new Audio(*_config)), 
    _network(new Network()),
    _camera(new Camera())
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

Game::~Game()
{
    delete _ball.release();
    delete _localPlayer.release();

    NewtonMaterialDestroyAllGroupID(_world);
    NewtonDestroyAllBodies(_world);
    NewtonDestroy(_world);
}

void Game::Run()
{
    clog << "Starting game..." << endl;

    // TODO: DANGER, textures can render incorrectly..
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    //

    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    //glDisable(GL_LIGHT0);
    //glDisable(GL_LIGHTING);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    Timer timer;

    int    frames = 0;
    double accum = 0.0;
    double currentTime = timer.Read();
    double startTime = currentTime;

    bool running = true;

    bool previous_active = true;
    
    while (running)
    {
        double newTime = timer.Read();
        double deltaTime = newTime - currentTime;
        currentTime = newTime;
        accum += deltaTime;

        if (glfwGetWindowParam(GLFW_ACTIVE)==GL_TRUE)
        {
            Control(deltaTime);
        }
    
        while (accum >= DT)
        {
            Update();
            accum -= DT;
        }
        Prepare();
        Render();
        frames++;

        glfwPollEvents();
        
        if (glfwGetWindowParam(GLFW_ACTIVE)==GL_FALSE && previous_active)
        {
            previous_active = false;
        }
        else if (glfwGetWindowParam(GLFW_ACTIVE)==GL_TRUE && !previous_active)
        {
            glfwRestoreWindow();
            previous_active = true;
        }

        running = glfwGetKey(GLFW_KEY_ESC)!=GLFW_PRESS;

    }

    clog << "Game finished... " << endl;
    clog << "Rendered " << frames << " frames in " << currentTime << " seconds = " 
         << frames/currentTime << " FPS" << endl;
}

void Game::Control(float delta)
{
    // read input from keyboard

    _camera->Control(delta);
    _localPlayer->Control();
}

void Game::Update()
{
    // update world objects, simulate physics

    NewtonUpdate(_world, DT);
}

void Game::Prepare()
{
    // prepare for rendering - store all object state in temporary variables

    _camera->Prepare();
    _localPlayer->Prepare();
    _ball->Prepare();

}

void Game::Render() const
{
    // render teh world, state of objects are in temporary variables

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _camera->Render();

    _video->RenderAxes();

    _localPlayer->Render(_video.get());
    _ball->Render(_video.get());

    glfwSwapBuffers();
}
