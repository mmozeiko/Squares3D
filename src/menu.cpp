#include <GL/glfw.h>
#include "openal_includes.h"

#include "menu.h"
#include "menu_options.h"
#include "menu_entries.h"
#include "menu_submenu.h"

#include "game.h"
#include "world.h"
#include "font.h"
#include "video.h"
#include "texture.h"
#include "geometry.h"
#include "input.h"
#include "language.h"
#include "config.h"
#include "timer.h"
#include "profile.h"
#include "colors.h"
#include "network.h"

// BUGBUG: ugly ugly windows.h include defines:
#undef min
#undef max

#include "audio.h"
#include "sound.h"
#include "music.h"
#include "sound_buffer.h"

class BoolValue : public Value
{
public:
    BoolValue(const string& id) : Value(id)
    {
        Value::add(Language::instance->get(TEXT_FALSE));
        Value::add(Language::instance->get(TEXT_TRUE));
    }
};

Menu::Menu(Profile* userProfile, int unlockable, int& current) :
    m_font(Font::get("Arial_32pt_bold")),
    m_fontBig(Font::get("Arial_72pt_bold")),
    m_state(State::Current),
    m_music(NULL),
    m_sound(new Sound(true)),
    m_sound2(new Sound(true)),
    m_soundOver(NULL),
    m_soundClick(NULL),
    m_soundBackClick(NULL),
    m_soundChange(NULL),
    m_mousePrevPos(Vector(static_cast<float>(Video::instance->getResolution().first/2),
                          0.0f, 
                          static_cast<float>(Video::instance->getResolution().second/2)))
{
    float resX = static_cast<float>(Video::instance->getResolution().first);
    float resY = static_cast<float>(Video::instance->getResolution().second);

    m_fontSmall = Font::get( ( resY <= 600 ? "Arial_12pt_bold" : "Arial_16pt_bold" ));

    const float aspect = (1.0f - resY/resX)/2.0f;

    m_backGround = new Face();
    m_backGround->vertexes.push_back(Vector::Zero);
    m_backGround->vertexes.push_back(Vector(0.0f, resY, 0.0f));
    m_backGround->vertexes.push_back(Vector(resX, 0.0f, 0.0f));
    m_backGround->vertexes.push_back(Vector(resX, resY, 0.0f));

    m_backGround->uv.push_back(UV(0.0f, aspect));
    m_backGround->uv.push_back(UV(0.0f, 1.0f - aspect));
    m_backGround->uv.push_back(UV(1.0f, aspect));
    m_backGround->uv.push_back(UV(1.0f, 1.0f - aspect));

    m_backGroundTexture = Video::instance->loadTexture("background", false);
    m_backGroundTexture->setFilter(Texture::Bilinear);

    loadMenu(userProfile, unlockable, current);
    Input::instance->startButtonBuffer();
    Input::instance->startKeyBuffer();
    Input::instance->startCharBuffer();
    Input::instance->mouseVisible(true);

    m_soundOver = Audio::instance->loadSound("menu_over");
    m_soundClick = Audio::instance->loadSound("menu_click");
    m_soundBackClick = Audio::instance->loadSound("menu_back");
    m_soundChange =Audio::instance->loadSound("menu_change");

    m_music = Audio::instance->loadMusic("menu_soundtrack");
    m_music->play();

    Network::instance->m_inMenu = true;
}

void Menu::loadMenu(Profile* userProfile, int unlockable, int& current)
{
    Language* language = Language::instance;

    int resX = Video::instance->getResolution().first;
    int resY = Video::instance->getResolution().second;

    Vector submenuPosition = Vector(static_cast<float>(resX) / 2,
                                    static_cast<float>(resY) / 4 * 3 / 2, 
                                    0);

    Vector titlePos = Vector(static_cast<float>(resX) / 2, 0, 0);
    float titleY = 0.0f;

    // Main Submenu

    Submenu* submenu = new Submenu(this);

    submenu->setTitle(language->get(TEXT_MAIN_MENU), titlePos);

    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_START_SINGLEPLAYER), false, "startSingle"));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_START_MULTIPLAYER), false, "startMulti"));
    submenu->m_entries.back()->disable();
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_PLAYER_OPTIONS), false, "playerOptions"));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_OPTIONS), false, "options"));
	submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_RULES), false, "rules"));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_CONTROLS), false, "controls"));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_CREDITS), false, "credits"));
    submenu->addEntry(new QuitEntry(this, language->get(TEXT_QUIT_GAME)));

    m_currentSubmenu = submenu;
    submenu->center(submenuPosition);
    m_submenus["main"] = submenu;


    // Start Game Submenu

    submenu = new Submenu(this);

    submenu->setTitle(language->get(TEXT_START_SINGLEPLAYER), titlePos);

    submenu->addEntry(new WorldEntry(this, language->get(TEXT_EASY), 0, current));
    submenu->addEntry(new WorldEntry(this, language->get(TEXT_NORMAL), 1, current));
    if (unlockable < 1)
    {
        submenu->m_entries.back()->disable();
    }
    submenu->addEntry(new WorldEntry(this, language->get(TEXT_HARD), 2, current));
    if (unlockable < 2)
    {
        submenu->m_entries.back()->disable();
    }
    submenu->addEntry(new WorldEntry(this, language->get(TEXT_EXTRA), 3, current));
    if (unlockable < 3)
    {
        submenu->m_entries.back()->disable();
    }
    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "main"));

    submenu->center(submenuPosition);
    m_submenus["startSingle"] = submenu;



    // Start Multi Join IP

    Submenu* submenuJ = new Submenu(this);

    submenuJ->setTitle(language->get(TEXT_START_JOIN), titlePos);

    submenuJ->addEntry(new WritableEntry(this, language->get(TEXT_ADDRESS), Config::instance->m_misc.last_address, submenuJ, 15));
    submenuJ->addEntry(new ConnectEntry(this, language->get(TEXT_CONNECT_HOST), "startMultiJoin", submenuJ));

    Network::instance->setMenuEntries(this, "startMultiJoin", "startMulti");

    submenuJ->addEntry(new SpacerEntry(this));
    submenuJ->addEntry(new CloseHostEntry(this, language->get(TEXT_BACK), "startMulti"));

    // TODO: hack
    submenuJ->m_height += 10;

    submenuJ->center(submenuPosition);
    m_submenus["startMultiJoinIP"] = submenuJ;



    // Start Multi Host

    submenu = new Submenu(this);

    submenu->setTitle(language->get(TEXT_START_HOST), titlePos);

    submenu->addEntry(new LabelEntry(this, language->get(TEXT_PLAYERS_LIST)));
    submenu->m_entries.back()->disable();
    submenu->addEntry(new NetPlayerEntry(this, 0));
    submenu->addEntry(new NetPlayerEntry(this, 1));
    submenu->addEntry(new NetPlayerEntry(this, 2));
    submenu->addEntry(new NetPlayerEntry(this, 3));

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new MultiWorldEntry(this, language->get(TEXT_BEGIN_GAME)));
    submenu->addEntry(new CloseHostEntry(this, language->get(TEXT_BACK), "startMulti"));

    submenu->center(submenuPosition);
    m_submenus["startMultiHost"] = submenu;


    // Start Multi Join
    submenu = new Submenu(this);

    submenu->setTitle(language->get(TEXT_START_JOIN), titlePos);

    submenu->addEntry(new LabelEntry(this, language->get(TEXT_PLAYERS_LIST)));
    submenu->m_entries.back()->disable();
    submenu->addEntry(new NetRemotePlayerEntry(this, 0));
    submenu->addEntry(new NetRemotePlayerEntry(this, 1));
    submenu->addEntry(new NetRemotePlayerEntry(this, 2));
    submenu->addEntry(new NetRemotePlayerEntry(this, 3));

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new CloseHostEntry(this, language->get(TEXT_DISCONNECT), "startMulti"));

    submenu->center(submenuPosition);
    m_submenus["startMultiJoin"] = submenu;


    // Credits
    submenu = new Submenu(this);
    
    submenu->setTitle(language->get(TEXT_CREDITS), titlePos);

    submenu->addEntry(new LabelEntry(this, language->get(TEXT_CREDITS_SCREEN), Font::Align_Left, Font::get("Arial_32pt_bold") ));

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "main"));    

    // TODO: hack
    submenu->m_height += 10;

    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["credits"] = submenu;

	// Rulez
    submenu = new Submenu(this);
    
    submenu->setTitle(language->get(TEXT_RULES), titlePos);

    submenu->addEntry(new LabelEntry(this, language->get(TEXT_RULES_SCREEN), Font::Align_Left, m_fontSmall));

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "main"));

    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["rules"] = submenu;

	// Controls
    submenu = new Submenu(this);
    
    submenu->setTitle(language->get(TEXT_CONTROLS), titlePos);

    const Font* controlsFont = Font::get( ( resY <= 600 ? "Arial_16pt_bold" : "Arial_32pt_bold" ));
    submenu->addEntry(new LabelEntry(this, language->get(TEXT_CONTROLS_SCREEN), Font::Align_Left, controlsFont));

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "main"));

    // TODO: hack
    submenu->m_height += 10;

    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["controls"] = submenu;


    // Start Multi Submenu

    submenu = new Submenu(this);

    submenu->setTitle(language->get(TEXT_START_MULTIPLAYER), titlePos);

    submenu->addEntry(new NewHostEntry(this, language->get(TEXT_START_HOST), "startMultiHost"));
    submenu->addEntry(new JoinHostEntry(this, language->get(TEXT_START_JOIN), "startMultiJoinIP", submenuJ));
    
    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "main"));

    // TODO: hack
    submenu->m_height += 10;

    submenu->center(submenuPosition);
    m_submenus["startMulti"] = submenu;



    // PLAYER Options Submenu
    submenu = new Submenu(this);
    
    submenu->setTitle(language->get(TEXT_PLAYER_OPTIONS), titlePos);

    submenu->addEntry(new WritableEntry(this, language->get(TEXT_NAME), userProfile->m_name, submenu));

    ColorValue colorValue("color");
    submenu->addEntry(new ColorEntry(this, language->get(TEXT_COLOR), userProfile->m_color, colorValue));

    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "main"));

    // TODO: hack
    submenu->m_height += 10;

    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["playerOptions"] = submenu;

    // Options Submenu
    submenu = new Submenu(this);
    
    submenu->setTitle(language->get(TEXT_OPTIONS), titlePos);

    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_VIDEO_OPTIONS), false, "videoOptionsEasy"));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_AUDIO_OPTIONS), false, "audioOptions"));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_OTHER_OPTIONS), false, "otherOptions"));

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "main"));    

    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["options"] = submenu;


    // VIDEO Options NOOBS Submenu
    submenu = new Submenu(this);
    
    submenu->setTitle(language->get(TEXT_VIDEO_OPTIONS), titlePos);

    IntPairVector resolutions = Video::instance->getModes();
    Value valueRes("resolution");
    for (size_t i = 0; i < resolutions.size(); i++)
    {
        int x = resolutions[i].first;
        int y = resolutions[i].second;
        wstring mode = wcast<wstring>(x) + L"x" + wcast<wstring>(y);
        if (x*3 == y*4)
        {
            mode += L" (4:3)";
        }
        else if (x*4 == y*5)
        {
            mode += L" (5:4)";
        }
        else if (x*9 == y*16)
        {
            mode += L" (16:9)";
        }
        else if (x*10 == y*16)
        {
            mode += L" (16:10)";
        }
        valueRes.add(mode);
    }
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_RESOLUTION), valueRes));

    BoolValue valFS("fullscreen");
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_FULLSCREEN), valFS));

    Value valEnvDet("environment_details");
    valEnvDet.add(language->get(TEXT_LOW));
    valEnvDet.add(language->get(TEXT_MEDIUM));
    valEnvDet.add(language->get(TEXT_HIGH));
    valEnvDet.add(language->get(TEXT_CUSTOM));
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_ENVIRONMENT_DETAILS), valEnvDet));

    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_VIDEO_OPTIONS_ADVANCED), false, "videoOptions"));

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new ApplyOptionsEntry(this, language->get(TEXT_SAVE), "videoOptionsEasy"));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "options"));    
   
    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["videoOptionsEasy"] = submenu;

    // VIDEO Options ADVANCED Submenu
    submenu = new Submenu(this);
    
    submenu->setTitle(language->get(TEXT_VIDEO_OPTIONS_ADVANCED), titlePos);

    BoolValue valVS("vsync");
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_VSYNC), valVS));

    Value valFSAA("fsaa_samples");
    valFSAA.add(L"0");
    valFSAA.add(L"2");
    valFSAA.add(L"4");
    valFSAA.add(L"6");
    valFSAA.add(L"8");
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_FSAA), valFSAA));

    Value valAn("anisotropy");
    valAn.add(L"0");
    for (int i=1; i<=Video::instance->m_maxAnisotropy; i++)
    {
        valAn.add(wcast<wstring>(1<<i));
    }
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_ANISOTROPY), valAn));
    if (Video::instance->m_haveAnisotropy == false)
    {
        submenu->m_entries.back()->disable();
    }

    BoolValue valShad("shadow_type");
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_SHADOW_TYPE), valShad));

    Value valShadS("shadowmap_size");
    valShadS.add(language->get(TEXT_LOW));
    valShadS.add(language->get(TEXT_MEDIUM));
    valShadS.add(language->get(TEXT_HIGH));
    if (!Video::instance->m_haveShadows)
    {
        valShadS.add(language->get(TEXT_NOT_SUPPORTED));
    }
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_SHADOWMAP_SIZE), valShadS));

	Value valHDR("use_hdr");
	valHDR.add(language->get(TEXT_FALSE));
	valHDR.add(language->get(TEXT_TRUE));
	if (!Video::instance->m_haveShaders)
    {
        valHDR.add(language->get(TEXT_NOT_SUPPORTED));
    }
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_USE_HDR), valHDR));

    Value valGD("grass_density");
    valGD.add(language->get(TEXT_LOW));
    valGD.add(language->get(TEXT_MEDIUM));
    valGD.add(language->get(TEXT_HIGH));
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_GRASS_DENSITY), valGD));

    Value valTD("terrain_detail");
    valTD.add(language->get(TEXT_LOW));
    valTD.add(language->get(TEXT_MEDIUM));
    valTD.add(language->get(TEXT_HIGH));
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_TERRAIN_DETAIL), valTD));

    BoolValue valFPS("show_fps");
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_SHOW_FPS), valFPS));

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new ApplyOptionsEntry(this, language->get(TEXT_SAVE), "videoOptions"));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "videoOptionsEasy"));
    
    // TODO: hack
    submenu->m_height -= 20;

    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["videoOptions"] = submenu;


    // AUDIO Options Submenu
    submenu = new Submenu(this);
    
    submenu->setTitle(language->get(TEXT_AUDIO_OPTIONS), titlePos);

    BoolValue valAud("audio");
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_AUDIO), valAud));

    Value valMV("music_vol");
    for (int i=0; i<10; i++)
    {
        valMV.add(wcast<wstring>(i));
    }
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_MUSIC_VOL), valMV));

    Value valSV("sound_vol");
    for (int i=0; i<10; i++)
    {
        valSV.add(wcast<wstring>(i));
    }
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_SOUND_VOL), valSV));

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new ApplyOptionsEntry(this, language->get(TEXT_SAVE), "audioOptions"));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "options"));
    
    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["audioOptions"] = submenu;

    
    
    // OTHER Options Submenu
    submenu = new Submenu(this);
    
    submenu->setTitle(language->get(TEXT_OTHER_OPTIONS), titlePos);

    Value valLang("language");
    valLang.add(language->get(TEXT_LANG_ENGLISH)); // remember to update in menu_options.cpp
    valLang.add(language->get(TEXT_LANG_LATVIAN));
    valLang.add(language->get(TEXT_LANG_RUSSIAN));
    valLang.add(language->get(TEXT_LANG_GERMAN));
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_LANGUAGE), valLang));

    BoolValue valSysK("system_keys");
    submenu->addEntry(new OptionEntry(this, language->get(TEXT_SYSTEM_KEYS), valSysK));

    /*
    Value valMS("mouse_sensitivity");
    for (int i=2; i<10*2; i++)
    {
        wstring ws = wcast<wstring>(static_cast<float>(i)/2.0f);
        if (ws.find(L".") == wstring::npos)
        {
            ws.push_back(L'.');
            ws.push_back(L'0');
        }
        valMS.add(ws);
    }
    submenu->addEntry(new OptionEntry(language->get(TEXT_MOUSE_SENSITIVITY), valMS));
    */

    submenu->addEntry(new SpacerEntry(this));
    submenu->addEntry(new ApplyOptionsEntry(this, language->get(TEXT_SAVE), "otherOptions"));
    submenu->addEntry(new SubmenuEntry(this, language->get(TEXT_BACK), true, "options"));

    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["otherOptions"] = submenu;

    titleY += m_font->getHeight() / 2;
    titleY = std::min(titleY, resY - static_cast<float>(m_fontBig->getHeight()));
    
    for each_(Submenus, m_submenus, iter)
    {
        if (!iter->second->m_title.empty())
        {
            iter->second->m_titlePos.y = titleY;
        }
    }
}

Menu::~Menu()
{
    Network::instance->m_inMenu = false;

    delete m_sound;
    delete m_sound2;
    Audio::instance->unloadSound(m_soundOver);
    Audio::instance->unloadSound(m_soundClick);
    Audio::instance->unloadSound(m_soundBackClick);
    Audio::instance->unloadSound(m_soundChange);
    Audio::instance->unloadMusic(m_music);

    Input::instance->mouseVisible(false);
    Input::instance->endKeyBuffer();
    Input::instance->endCharBuffer();
    Input::instance->endButtonBuffer();

    for each_const(Submenus, m_submenus, iter)
    {
        delete iter->second;
    }

    delete m_backGround;
}

State::Type Menu::progress()
{
    return (Network::instance->m_needToStartGame ? State::World : m_state);
}

void Menu::setState(State::Type state)
{
     m_state = state;
}

void Menu::setSubmenu(const string& submenuToSwitchTo)
{
    m_currentSubmenu = m_submenus.find(submenuToSwitchTo)->second;
    
    //highlight the first non disabled entry from above
    m_currentSubmenu->m_activeEntry = m_currentSubmenu->m_entries.size() - 1;
    m_currentSubmenu->activateNextEntry(true);

    Entries& entries = m_submenus[submenuToSwitchTo]->m_entries;
    for each_(Entries, entries, iter)
    {
        (*iter)->reset();
    }

    if (submenuToSwitchTo == "startMulti")
    {
        Entries& entries = m_submenus["startMultiJoinIP"]->m_entries;

        for each_(Entries, entries, iter)
        {
            (*iter)->enable();
        }

        entries[1]->m_string = Language::instance->get(TEXT_CONNECT_HOST);
    }
}

void Menu::control()
{
    int key;
    bool done = false;
    while (!done)
    {
        key = Input::instance->popKey();
        if (key < 32 || key >= 127)
        {
            if (key == GLFW_KEY_ESC)
            {
                if (m_currentSubmenu == m_submenus["main"])
                {
                    m_state = State::Quit;
                }
                else
                {
                    // assume "back" is last entry
                    size_t i = m_currentSubmenu->m_entries.size()-1;
                    while (m_currentSubmenu->m_entries[i]->isEnabled() == false)
                    {
                        i--;
                    }
                    m_currentSubmenu->m_entries[i]->click(GLFW_KEY_ENTER);
                }
            }
            m_currentSubmenu->control(key);
        }
        done = key==-1;
    }

    done = false;
    while (!done)
    {
        int ch = Input::instance->popChar();
        m_currentSubmenu->onChar(ch);
        done = ch==-1;
    }
}

void Menu::render() const
{
    m_font->begin();

    m_backGroundTexture->bind();
    glDisableClientState(GL_NORMAL_ARRAY);
    Video::instance->renderFace(*m_backGround);
    glEnableClientState(GL_NORMAL_ARRAY);

    m_font->begin2();
    m_currentSubmenu->render();
    m_font->end();
}
