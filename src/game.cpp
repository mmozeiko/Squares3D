#include "game.h"
#include "timer.h"

#include "config.h"
#include "video.h"
#include "audio.h"
#include "network.h"
#include "input.h"
#include "world.h"
#include "camera.h"
#include "font.h"
#include "fps.h"

#include "vmath.h"

Game::Game()
{
    m_config = new Config();
    m_video = new Video(this);
    m_audio = new Audio(this);
    m_network = new Network();
    m_input = new Input();
    m_world = new World(this);

    m_world->init();
}

Game::~Game()
{
    delete m_world;
    delete m_input;
    delete m_network;
    delete m_audio;
    delete m_video;
    delete m_config;
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

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

	GLfloat lightColor[] = { 0.8f, 0.8f, 0.8f, 1.0 };
	GLfloat lightAmbientColor[] = { 0.5f, 0.5f, 0.5f, 1.0 };
	GLfloat lightPosition[] = { 500.0f, 200.0f, 500.0f, 0.0 };
	
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbientColor);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    Timer timer;

    double accum = 0.0;
    double currentTime = timer.read();
    double startTime = currentTime;

    bool running = true;

    bool previous_active = true;
    
    Font font("Arial_32pt_bold.bff");
    FPS fps(timer, font);

    while (running)
    {
        double newTime = timer.read();
        double deltaTime = newTime - currentTime;
        currentTime = newTime;
        
        if (deltaTime > 0.01f) deltaTime = 0.01f; // TODO: REMOVE!!!!!

        accum += deltaTime;

        m_audio->update();
        m_input->update();
        m_world->control(m_input);
        m_world->m_camera->update(static_cast<float>(deltaTime));
        while (accum >= DT)
        {
            m_world->update(DT);
            accum -= DT;
        }
        m_world->prepare();
        m_world->render(m_video);

        fps.update();
        fps.render();

        glfwSwapBuffers();
        glfwPollEvents();
        
        // glfw minmize/restore focus bug
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
    clog << "Rendered " << fps.frames() << " frames in " << fps.time() << " seconds = " << fps.fps() << " FPS" << endl;
}

