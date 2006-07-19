#include "game.h"
#include "timer.h"

#include "config.h"
#include "video.h"
#include "audio.h"
#include "network.h"
#include "input.h"
#include "world.h"
#include "camera.h"

#include "vmath.h"

Game::Game() : m_config(new Config())
{
    m_video.reset(new Video(this));
    m_audio.reset(new Audio(this));
    m_network.reset(new Network());
    m_input.reset(new Input());
    m_world.reset(new World(this));
    
    m_world->init();
}

Game::~Game()
{
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

	GLfloat lightColor[] = { 0.7f, 0.7f, 0.7f, 1.0 };
	GLfloat lightAmbientColor[] = { 0.5f, 0.5f, 0.5f, 1.0 };
	GLfloat lightPoition[] = { 500.0f, 200.0f, 500.0f, 0.0 };
	
	glLightfv(GL_LIGHT0, GL_POSITION, lightPoition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbientColor);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);

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
        // TODO: REMOVE!!!!!
        if (deltaTime > 0.01f) deltaTime = 0.01f;
        accum += deltaTime;

        m_input->process();
        m_world->control(m_input.get());
        m_world->m_camera->update(static_cast<float>(deltaTime));
        while (accum >= DT)
        {
            m_world->update(DT);
            accum -= DT;
        }
        m_world->prepare();
        m_world->render(m_video.get());
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
    clog << "Rendered " << frames << " frames in " << (currentTime-startTime) << " seconds = " 
         << frames/(currentTime-startTime) << " FPS" << endl;
}

