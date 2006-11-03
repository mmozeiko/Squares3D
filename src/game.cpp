#include <physfs.h>

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
#include "random.h"

template <class Game> Game* System<Game>::instance = NULL;

static const string USER_PROFILE_FILE = "/user.xml";

static const unsigned int M1 = 0x0BADC0DE;
static const unsigned int M2 = 0xDEADBEEF;

bool   g_needsToReload = false;
string g_optionsEntry;

Game::Game() : 
    m_fixedTimestep(true),
    m_unlockable(-1),
    m_current(0),
    m_userProfile(NULL),
    m_state(NULL),
    m_screenLast(false)
{
    // these and only these objects are singletons,
    // they all have public static instance attribute
    m_config = new Config();
    m_input = new Input();
    m_language = new Language();
    m_video = new Video();
    m_audio = new Audio();
    m_network = new Network();
	m_input->init();
    //

    loadUserData();
    loadCpuData();

    if (g_needsToReload)
    {
        g_needsToReload = false;

        Menu* menu = new Menu(m_userProfile, m_unlockable, m_current);
        menu->setSubmenu(g_optionsEntry);
        m_state = menu;
    }
    else
    {
        m_state = new Intro();
        m_fixedTimestep = false;
    }    
    m_state->init();
}

Game::~Game()
{
    Font::unload();
    
    saveUserData();
 
    for (size_t i = 0; i < 4; i++)
    {
        for each_const(ProfilesVector, m_cpuProfiles[i], iter)
        {
            delete *iter;
        }
    }

    if (m_state != NULL)
    {
        delete m_state;
    }

    delete m_network;
    delete m_audio;
    delete m_video;
    delete m_language;
    delete m_input;
    delete m_config;
}

static const int BMP_BITS_OFF = 54;

static unsigned char header[] = {
    'B', 'M',
    0, 0, 0, 0,  // size
    0, 0, 0, 0,  // reserved
    BMP_BITS_OFF, 0, 0, 0,  // offset to bits
    40, 0, 0, 0, // header size
    0, 0, 0, 0,  // width
    0, 0, 0, 0,  // height
    1, 0,        // planes
    24, 0,       // bits
    0, 0, 0, 0,  // compression
    0, 0, 0, 0,  // data size
    0, 0, 0, 0,  // hres
    0, 0, 0, 0,  // vres
    0, 0, 0, 0,  // color count
    0, 0, 0, 0,  // important count
};

void Game::saveScreenshot(const FPS& fps) const
{
    int x = Video::instance->getResolution().first;
    int y = Video::instance->getResolution().second;

    *reinterpret_cast<unsigned int*>(&header[2]) = BMP_BITS_OFF + x*y*3;
    *reinterpret_cast<unsigned int*>(&header[18]) = x;
    *reinterpret_cast<unsigned int*>(&header[22]) = y;

    vector<unsigned char> data(x*y*3);

    string frames = cast<string>(fps.frames());
    while (frames.size() < 4)
    {
        frames = '0' + frames;
    }
    
    static const int GL_BGR__ = 0x80E0;
    glReadPixels(0, 0, x, y, GL_BGR__, GL_UNSIGNED_BYTE, &data[0]);

    FILE* f = fopen(("screenshot_" + frames + ".bmp").c_str(), "wb");
    fwrite(header, sizeof(header), 1, f);
    fwrite(&data[0], x*y*3, 1, f);
    fclose(f);
}

void Game::run()
{
    clog << "Starting game..." << endl;

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

        float newTime = timer.read();
        float deltaTime = newTime - currentTime;
        currentTime = newTime;
        
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

//#ifndef NDEBUG
        if (Input::instance->key('`'))
        {
            glPolygonMode(GL_FRONT, GL_LINE);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        else
        {
            glPolygonMode(GL_FRONT, GL_FILL);
        }
//#endif
        m_state->render();
        
        fps.update();

        if (Config::instance->m_video.show_fps)
        {
            fps.render();
        }
        glfwSwapBuffers();

        if (Input::instance->key('S') && m_screenLast==false)
        {
            saveScreenshot(fps);
            m_screenLast = true;
        }
        if (m_screenLast && Input::instance->key('S')==false)
        {
            m_screenLast = false;
        }

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

    /*
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
    */

    clog << "Game finished... " << endl;
    //clog << "Rendered " << fps.frames() << " frames in " << fps.time() << " seconds = " << fps.fps() << " FPS" << endl;
}

State* Game::switchState(const State::Type nextState)
{
    m_fixedTimestep = true;
    switch (nextState)
    {
    case State::Intro:
        m_fixedTimestep = false;
        return new Intro();
    case State::Menu  : return new Menu(m_userProfile, m_unlockable, m_current);
    case State::World : return new World(m_userProfile, m_unlockable, m_current);
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
                    if (node.name == "magic")
                    {
                        unsigned int magic1 = node.getAttribute<unsigned int>("magic1");
                        unsigned int magic2 = node.getAttribute<unsigned int>("magic2");
                        unsigned int magic3 = node.getAttribute<unsigned int>("magic3");
                        unsigned int magic4 = node.getAttribute<unsigned int>("magic4");
                        
                        m_unlockable = ((magic1 + magic2 + M1) ^ magic4) - magic3 - M2;

                        if (m_unlockable < 0 || m_unlockable > 3)
                        {
                            m_unlockable = 0;
                        }
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
        if ((m_unlockable == -1) || (m_userProfile == NULL))
        {
            throw Exception("Corrupted user profile file!");
        }
    }
    else
    {
        m_userProfile = new Profile();
        m_unlockable = 0;
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
    other_data.childs.push_back(XMLnode("magic"));
    
    unsigned int magic1 = Randoms::getInt();
    unsigned int magic2 = Randoms::getInt();
    unsigned int magic3 = Randoms::getInt();
    unsigned int magic4 = (magic1 + magic2 + M1) ^ (magic3 + static_cast<unsigned int>(m_unlockable) + M2);
    other_data.childs.back().setAttribute("magic1", cast<string>(magic1));
    other_data.childs.back().setAttribute("magic2", cast<string>(magic2));
    other_data.childs.back().setAttribute("magic3", cast<string>(magic3));
    other_data.childs.back().setAttribute("magic4", cast<string>(magic4));

    File::Writer out(USER_PROFILE_FILE);
    if (!out.is_open())
    {
        throw Exception("Failed to open " + USER_PROFILE_FILE + " for writing");
    }
    xml.save(out);
    out.close();
    delete m_userProfile;
}

void Game::loadCpuData()
{
    XMLnode xml;
    File::Reader in("/data/level/cpu_players.xml");
    if (!in.is_open())
    {
        throw Exception("Level file '/data/level/cpu_players.xml' not found");  
    }
    xml.load(in);
    in.close();
    int checks[4] = {0,0,0,0};

    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;
        if ((node.name == "easy") || (node.name == "normal") || (node.name == "hard") || (node.name == "extra"))
        {
            size_t idx;
            if (node.name == "easy")
            {
                idx = 0;
            }
            else if (node.name == "normal")
            {
                idx = 1;
            }
            else if (node.name == "hard")
            {
                idx = 2;
            }
            else // if (node.name == "extra")
            {
                idx = 3;
            }

            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "profile")
                {
                    Profile* profile = new Profile(node);
                    m_cpuProfiles[idx].push_back(profile);
                    checks[idx]++;
                }
                else
                {
                    throw Exception("Invalid profile, unknown node - " + node.name);
                }
            }
        }
        else
        {
            throw Exception("Invalid cpu_profiles, unknown node - " + node.name);
        }
    }

    for (size_t i = 0; i < 4; i++)
    {
        if (checks[i] < 3)
        {
            throw Exception("Invalid cpu_profiles, there should be at least 3 profiles in each difficulty");
        }
    }
}
