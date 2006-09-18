#include <cstdio>
#include <PhysFS.h>

#include "game.h"
#include "timer.h"
#include "config.h"
#include "language.h"
#include "video.h"
#include "audio.h"
#include "network.h"
#include "input.h"
#include "world.h"
#include "menu.h"
#include "intro.h"
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
    // these and only these objects are singletons,
    // they all have public static instance attribute
    m_config = new Config();
    m_language = new Language();
    m_video = new Video();
    m_audio = new Audio();
    m_network = new Network();
    m_input = new Input();
    //

    m_state = new Intro();

    m_network->createClient();
    m_network->connect("localhost");
}

Game::~Game()
{
    Font::unload();

    delete m_state;

    delete m_input;
    delete m_network;
    delete m_audio;
    delete m_video;
    delete m_language;
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

    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_NORMALIZE);

    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    bool running = true;
    bool previous_active = true;
    
    const Font* font = Font::get("Arial_32pt_bold");

    Timer timer;
    FPS fps(timer, font);

    float accum = 0.0f;
    float currentTime = timer.read();
    float startTime = currentTime;

    while (running)
    {
        m_audio->update();
        m_input->update();
        m_network->update();

        m_state->control();

#ifdef MAKE_MOVIE
        float deltaTime = 1.0f/25.0f;
#else
        float newTime = timer.read();
        float deltaTime = newTime - currentTime;
        currentTime = newTime;
#endif
        
        //if (deltaTime > 0.01f) deltaTime = 0.01f; // TODO: REMOVE!!!!!

        accum += deltaTime;

        m_state->update(accum);

#ifndef FIXED_TIMESTEP
        m_state->updateStep(accum);
        accum = 0.0f;
#else
        if (accum >= 4*DT)
        {
            // hmmm.. not a nice HACK.
            while (accum >= 4*DT)
            {
                m_state->updateStep(4*DT);
                accum -= 4*DT;
            }
            m_state->updateStep(accum);
            accum = 0.0f;
        }
        else
        {
            while (accum >= DT)
            {
                m_state->updateStep(DT);
                accum -= DT;
            }
        }
#endif

        m_state->prepare();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_state->render();

        fps.update();

#ifndef MAKE_MOVIE
        if (Config::instance->m_video.show_fps)
        {
            fps.render();
        }
#endif

        glfwSwapBuffers();

#ifdef MAKE_MOVIE
        saveScreenshot(fps);
#endif

        State::Type newState = m_state->progress();

        if (newState == State::Quit)
        {
            running = false;
        }
        else if (newState != State::Current)
        {
            delete m_state;
            timer.reset();
            fps.reset();
            currentTime = accum = 0.0f;
            m_state = switchState(newState);
        }
        
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

State* Game::switchState(const State::Type nextState) const
{
    switch (nextState)
    {
    case State::Menu: return new Menu();
    case State::World: return new World();
    //TODO: implement these
    //case State_Intro: return ..;
    //case State_Lobby: return ..;
    default:
        assert(false);
        return NULL;
    }
}
