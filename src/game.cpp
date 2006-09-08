#include <cstdio>
#include <PhysFS.h>

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

#define FIXED_TIMESTEP
//#define MAKE_MOVIE
#define MOVIE_WIDTH 640
#define MOVIE_HEIGHT 480

Game::Game()
{
    m_config = new Config();
    m_video = new Video();
    m_audio = new Audio();
    m_network = new Network();
    m_input = new Input();
    m_world = new World();

    m_world->init();
    m_network->createClient();
    m_network->connect("localhost");
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

#ifdef MAKE_MOVIE

#define BMP_BITS_OFF 54
#define BMP_SIZE (BMP_BITS_OFF + MOVIE_HEIGHT*MOVIE_WIDTH*3)

static const unsigned char header[] = {
    'B', 'M',
    BMP_SIZE&0xFF, (BMP_SIZE>>8)&0xFF, (BMP_SIZE>>16)&0xFF, 0,  // size
    0, 0, 0, 0,  // reserved
    BMP_BITS_OFF, 0, 0, 0,  // offset to bits
    40, 0, 0, 0, // header size
    MOVIE_WIDTH&0xFF, MOVIE_WIDTH>>8, 0, 0,  // width
    MOVIE_HEIGHT&0xFF, MOVIE_HEIGHT>>8, 0, 0,  // height
    1, 0,        // planes
    24, 0,       // bits
    0, 0, 0, 0,  // compression
    0, 0, 0, 0,  // data size
    0, 0, 0, 0,  // hres
    0, 0, 0, 0,  // vres
    0, 0, 0, 0,  // color count
    0, 0, 0, 0,  // important count
};

static unsigned char data[MOVIE_WIDTH*MOVIE_HEIGHT*3];

#endif

void Game::saveScreenshot(const FPS& fps) const
{
#ifdef MAKE_MOVIE
    string frames = cast<string>(fps.frames());
    while (frames.size() < 4)
    {
        frames = '0' + frames;
    }
    
    glReadPixels(0, 0, MOVIE_WIDTH, MOVIE_HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, data);

    FILE* f = fopen(("movie/frame" + frames + ".bmp").c_str(), "wb");
    fwrite(header, sizeof(header), 1, f);
    fwrite(data, MOVIE_HEIGHT*MOVIE_WIDTH*3, 1, f);
    fclose(f);
#endif
}

void Game::run()
{
    clog << "Starting game..." << endl;
#ifdef MAKE_MOVIE
    PHYSFS_mkdir("movie");
#endif

    glFrontFace(GL_CW);
    //glEnable(GL_CULL_FACE); // TODO: hack
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

    float accum = 0.0f;
    float currentTime = timer.read();
    float startTime = currentTime;

    bool running = true;

    bool previous_active = true;
    
    Font font("Arial_32pt_bold.bff");
    FPS fps(timer, font);

    while (running)
    {
#ifdef MAKE_MOVIE
        float deltaTime = 1.0f/25.0f;
#else
        float newTime = timer.read();
        float deltaTime = newTime - currentTime;
        currentTime = newTime;
#endif
        
        //if (deltaTime > 0.01f) deltaTime = 0.01f; // TODO: REMOVE!!!!!

        accum += deltaTime;

        m_audio->update();
        m_input->update();
        m_network->update();
        m_world->control();

        m_world->update(accum);

#ifndef FIXED_TIMESTEP
        m_world->updateStep(accum);
        accum = 0.0f;
#else
        while (accum >= DT)
        {
            m_world->updateStep(DT);
            accum -= DT;
        }
#endif

        m_world->prepare();

        glClear(GL_DEPTH_BUFFER_BIT);
        m_world->render();

        fps.update();
#ifndef MAKE_MOVIE
        fps.render();
#endif

        glfwSwapBuffers();

#ifdef MAKE_MOVIE
        saveScreenshot(fps);
#endif
        
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

    // disconnect network
    if (m_network->m_needDisconnect)
    {
        // display some message "Closing network connection...
        // ...
        
        // and disconnect
        m_network->sendDisconnect();
        while (! m_network->m_disconnected )
        {
            m_network->update();
        }
    }

    clog << "Game finished... " << endl;
    clog << "Rendered " << fps.frames() << " frames in " << fps.time() << " seconds = " << fps.fps() << " FPS" << endl;
}

