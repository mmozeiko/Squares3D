#include "game.h"
#include "timer.h"
#include "player_local.h"

#include "vmath.h"

#include <GL/glfw.h>

#define DT         0.01f

struct ContactBodies
{
    const NewtonBody* body1;
    const NewtonBody* body2;
};

// this callback is called when the aabb box with player overlaps floor
int  PlayerContactBegin(const NewtonMaterial* material, const NewtonBody* body1, const NewtonBody* body2)
{
	ContactBodies* contactBodies = reinterpret_cast<ContactBodies*>(NewtonMaterialGetMaterialPairUserData(material));
    contactBodies->body1 = body1;
    contactBodies->body2 = body2;

    return 1;
}

static int PlayerContactProcess(const NewtonMaterial* material, const NewtonContact* contact)
{
	ContactBodies* contactBodies = reinterpret_cast<ContactBodies*>(NewtonMaterialGetMaterialPairUserData(material));
    
	Player* player;

    // TODO: ugly code, somebody please fix it!

    player = reinterpret_cast<Player*>(NewtonBodyGetUserData(contactBodies->body1));
    if (player)
    {
        player->OnCollision(material, contact);
    }
    else
    {
        player = reinterpret_cast<Player*>(NewtonBodyGetUserData(contactBodies->body2));
        if (player)
        {
            player->OnCollision(material, contact);
        }
    }

    // ** end of ugly code

    return 1;
}

ContactBodies contactBodies;

Game::Game() : 
    _config(), 
    _video(_config), 
    _audio(_config), 
    _network(),
    _camera()
{
    _world = NewtonCreate(NULL, NULL);
    NewtonWorldSetUserData(_world, reinterpret_cast<void*>(this));
    NewtonSetSolverModel(_world, 10);
    NewtonSetFrictionModel(_world, 1);

    NewtonCollision* floorCol = NewtonCreateBox(_world, 100.0f, 1.0f, 100.f, NULL);
    NewtonBody* floorBody = NewtonCreateBody(_world, floorCol);
	NewtonReleaseCollision(_world, floorCol);

    Matrix pos = Matrix::translate(Vector(0.0f, -0.5f, 0.0f));
    pos.transpose();
    NewtonBodySetMatrix(floorBody, pos.m);



	int floorID = NewtonMaterialCreateGroupID(_world);
	int charID = NewtonMaterialCreateGroupID(_world);

	NewtonMaterialSetDefaultElasticity(_world, floorID, charID, 0.4f);
	NewtonMaterialSetDefaultFriction(_world, floorID, charID, 0.4f, 0.4f);

	NewtonMaterialSetCollisionCallback(_world, floorID, charID, 
        static_cast<void*>(&contactBodies), 
        PlayerContactBegin, PlayerContactProcess, NULL); 

    NewtonBodySetMaterialGroupID(floorBody, floorID);



    _localPlayer = new LocalPlayer(_world, charID, Vector(0.0f, 2.0f, 0.0f), Vector(0.75f, 2.0f, 0.75f));
}

Game::~Game()
{
    delete _localPlayer;
    NewtonMaterialDestroyAllGroupID(_world);
    NewtonDestroy(_world);
}

void Game::run()
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
    double currentTime = timer.read();
    double startTime = currentTime;

    bool running = true;
    
    while (running)
    {
        double newTime = timer.read();
        double deltaTime = newTime - currentTime;
        currentTime = newTime;
        accum += deltaTime;

        control(deltaTime);
        while (accum >= DT)
        {
            update();
            accum -= DT;
        }
        prepare();
        render();
        frames++;

        glfwPollEvents();
        running = glfwGetKey(GLFW_KEY_ESC)!=GLFW_PRESS;

    }

    clog << "Game finished... " << endl;
    clog << "Rendered " << frames << " frames in " << currentTime << " seconds = " 
         << frames/currentTime << " FPS" << endl;
}

void Game::control(float delta)
{
    // read input from keyboard

    int w, h;
    glfwGetWindowSize(&w, &h);
    int x, y;
    glfwGetMousePos(&x, &y);
    int w2 = w/2, h2 = h/2;

    if (x!=w2 || y !=h2)
    {
        _camera.rotate(
            delta * M_PI * LOOK_SPEED * (y-h2) / h2,
            delta * M_PI * LOOK_SPEED * (x-w2) / w2);

        glfwSetMousePos(w2, h2);
    }
    float dist = 0.0f;
    float strafe = 0.0f;

    if (glfwGetKey(GLFW_KEY_UP)==GLFW_PRESS) dist += 1.0f;
    if (glfwGetKey(GLFW_KEY_DOWN)==GLFW_PRESS) dist -= 1.0f;
    if (glfwGetKey(GLFW_KEY_RIGHT)==GLFW_PRESS) strafe += 1.0f;
    if (glfwGetKey(GLFW_KEY_LEFT)==GLFW_PRESS) strafe -= 1.0f;

    if (dist!=0 || strafe!=0)
    {
        _camera.move(delta * MOVE_SPEED * dist, delta * MOVE_SPEED * strafe);
    }

    _localPlayer->control();

}

void Game::update()
{
    // update world objects, simulate physics
    NewtonUpdate(_world, DT);
}

void Game::prepare()
{
    // prepare for rendering - store all object state in temporary variables
    _camera.prepare();
    _localPlayer->prepare();

}

void Game::render() const
{
    // render teh world, state of objects are in temporary variables

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _camera.render();

    _video.renderAxes();

    _localPlayer->render(_video);

    glfwSwapBuffers();
}
