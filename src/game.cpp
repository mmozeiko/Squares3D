#include "game.h"
#include "timer.h"

#include "config.h"
#include "video.h"
#include "audio.h"
#include "network.h"
#include "input.h"
#include "world.h"

#include "vmath.h"

Game::Game() : 
    m_config(new Config()), 
    m_video(new Video(m_config.get())), 
    m_audio(new Audio(m_config.get())),
    m_network(new Network()),
    m_input(new Input())
{
    m_world.reset(new World(this));
}

Game::~Game()
{
}

const Video* Game::video() const
{
    return m_video.get();
}


void Game::run()
{
    clog << "Starting game..." << endl;

    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

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

    bool previous_active = true;
    
    while (running)
    {
        double newTime = timer.read();
        double deltaTime = newTime - currentTime;
        currentTime = newTime;
        accum += deltaTime;

        m_input->process();
        m_world->control(m_input.get(), static_cast<float>(deltaTime));
        while (accum >= DT)
        {
            m_world->update();
            accum -= DT;
        }
        m_world->prepare();
        m_world->render();
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

