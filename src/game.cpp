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
#include "profile.h"

#include "vmath.h"

//#define MAKE_MOVIE
#define MOVIE_WIDTH 640
#define MOVIE_HEIGHT 480

bool   g_needsToReload = false;
string g_optionsEntry;

Game::Game() : m_fixedTimestep(true)
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

    m_user = new Profile();

    if (g_needsToReload)
    {
        g_needsToReload = false;

        Menu* menu = new Menu(m_user);
        menu->setSubmenu(g_optionsEntry);
        m_state = menu;
    }
    else
    {
        m_state = new Intro();
        m_fixedTimestep = false;
    }
    m_state->init();

    m_network->createClient();
    m_network->connect("localhost");
}

Game::~Game()
{
    Font::unload();
    
    delete m_state;

    m_user->saveUserProfile();

    delete m_user;
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

    m_video->init();

    bool running = true;
    bool previous_active = true;
    
    const Font* font = Font::get("Arial_32pt_bold");

    Timer timer;
    FPS fps(timer, font);

    float accum = 0.0f;
    float currentTime = timer.read();

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

        if (m_fixedTimestep)
        {
            accum += deltaTime;

            m_state->update(accum - fmodf(accum, DT));

            while (accum >= DT)
            {
                m_state->updateStep(DT);
                accum -= DT;
            }
        }
        else
        {
            m_state->update(deltaTime);
            m_state->updateStep(deltaTime);
        }

        m_state->prepare();

#ifndef NDEBUG
        if (Input::instance->key('`'))
        {
            glPolygonMode(GL_FRONT, GL_LINE);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        else
        {
            glPolygonMode(GL_FRONT, GL_FILL);
        }
#endif
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
            m_state = switchState(newState);
            m_state->init();

            timer.reset();
            fps.reset();
            currentTime = accum = 0.0f;
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

        if (glfwGetWindowParam(GLFW_OPENED)==GL_FALSE)
        {
            running = false;
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

State* Game::switchState(const State::Type nextState)
{
    m_fixedTimestep = true;
    switch (nextState)
    {
    case State::Intro:
        m_fixedTimestep = false;
        return new Intro();
    case State::Menu: return new Menu(m_user);
    case State::World: return new World(m_user);
    //TODO: implement these
    //case State_Lobby: return ..;
    default:
        assert(false);
        return NULL;
    }
}
