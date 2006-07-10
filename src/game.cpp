#include "game.h"
#include "timer.h"

#include "config.h"
#include "video.h"
#include "audio.h"
#include "network.h"
#include "world.h"

#include "vmath.h"

Game::Game() : 
    _config(new Config()), 
    _video(new Video(*_config)), 
    _audio(new Audio(*_config)), 
    _network(new Network())
{
    _world.reset(new World(this));
}

Game::~Game()
{
}

const Video* Game::video() const
{
    return _video.get();
}


void Game::Run()
{
    clog << "Starting game..." << endl;

    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

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
            _world->Control(deltaTime);
        }
    
        while (accum >= DT)
        {
            _world->Update();
            accum -= DT;
        }
        _world->Prepare();
        _world->Render();
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

