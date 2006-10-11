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
#include "colors.h"
#include "xml.h"

const static string USER_PROFILE_FILE = "/user.xml";


//#define MAKE_MOVIE
#define MOVIE_WIDTH 640
#define MOVIE_HEIGHT 480

bool   g_needsToReload = false;
string g_optionsEntry;

Game::Game() : 
    m_fixedTimestep(true),
    m_difficulty(0),
    m_unlockedDifficulty(0),
    m_userProfile(NULL)
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

    loadUserData();

    if (g_needsToReload)
    {
        g_needsToReload = false;

        Menu* menu = new Menu(m_userProfile, &m_unlockables);
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

    saveUserData();

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
            m_state->m_current = newState;
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
    case State::MenuEasy   : 
    case State::MenuNormal : 
    case State::MenuHard   : if (m_unlockables.m_difficulty < (nextState - State::MenuEasy))
                             {
                                 //user has cleared higher level than before - unlock next difficulty
                                 m_unlockables.m_difficulty = nextState - State::MenuEasy;
                             }
                             return new Menu(m_userProfile, &m_unlockables);
    case State::WorldEasy  :
    case State::WorldNormal:
    case State::WorldHard  : return new World(m_userProfile, nextState - State::WorldEasy);
    default:
        assert(false);
        return NULL;
    }
}

void Game::loadUserData()
{
    clog << "Reading user information." << endl;
    XMLnode xml;
    File::Reader in(USER_PROFILE_FILE);
    if (in.is_open())
    {
        xml.load(in);
        in.close();
        for each_const(XMLnodes, xml.childs, iter)
        {
            const XMLnode& node = *iter;
            if (node.name == "profile")
            {
                if (m_userProfile != NULL)
                {
                    throw Exception("User profile should not be initialized twice!");
                }
                else
                {
                    m_userProfile = new Profile(node);
                }
            }
            else if (node.name == "other_data")
            {
                for each_const(XMLnodes, node.childs, iter)
                {
                    const XMLnode& node = *iter;
                    if (node.name == "difficulty")
                    {
                        m_unlockables.m_difficulty = cast<int>(node.getAttribute("unlocked"));
                    }
                    else
                    {
                        string line = cast<string>(node.line);
                        throw Exception("Invalid profile file, unknown profile parameter '" + node.name + "' at line " + line);
                    }
                }
            }
            else
            {
                string line = cast<string>(node.line);
                throw Exception("Invalid profile file, unknown profile parameter '" + node.name + "' at line " + line);
            }
        }
        if ((m_difficulty == -1) || (m_userProfile == NULL))
        {
            throw Exception("Corrupted user profile file!");
        }
    }
    else
    {
        m_userProfile = new Profile();
        m_difficulty = 0;
    }
}

void Game::saveUserData()
{
    clog << "Saving user data." << endl;

    XMLnode xml("xml");
    xml.childs.push_back(XMLnode("profile"));
    XMLnode& profile = xml.childs.back();
    profile.childs.push_back(XMLnode("name", m_userProfile->m_name));
    profile.childs.push_back(XMLnode("collision", m_userProfile->m_collisionID));
    profile.childs.push_back(XMLnode("properties"));
    profile.childs.back().setAttribute("speed", cast<string>(m_userProfile->m_speed));
    profile.childs.back().setAttribute("accuracy", cast<string>(m_userProfile->m_accuracy));
    profile.childs.back().setAttribute("jump", cast<string>(m_userProfile->m_jump));
    profile.childs.push_back(XMLnode("color"));
    profile.childs.back().setAttribute("r", cast<string>(m_userProfile->m_color.x));
    profile.childs.back().setAttribute("g", cast<string>(m_userProfile->m_color.y));
    profile.childs.back().setAttribute("b", cast<string>(m_userProfile->m_color.z));
    
    xml.childs.push_back(XMLnode("other_data"));
    XMLnode& other_data = xml.childs.back();
    other_data.childs.push_back(XMLnode("difficulty"));
    other_data.childs.back().setAttribute("unlocked", cast<string>(m_unlockables.m_difficulty));

    File::Writer out(USER_PROFILE_FILE);
    if (!out.is_open())
    {
        throw Exception("Failed to open " + USER_PROFILE_FILE + " for writing");
    }
    xml.save(out);
    out.close();
    delete m_userProfile;
}
