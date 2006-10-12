#include <GL/glfw.h>

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
    m_state(State::Current)
{
    float resX = static_cast<float>(Video::instance->getResolution().first);
    float resY = static_cast<float>(Video::instance->getResolution().second);

    m_backGround = new Face();
    m_backGround->vertexes.push_back(Vector::Zero);
    m_backGround->vertexes.push_back(Vector(0.0f, resY, 0.0f));
    m_backGround->vertexes.push_back(Vector(resX, 0.0f, 0.0f));
    m_backGround->vertexes.push_back(Vector(resX, resY, 0.0f));

    m_backGround->uv.push_back(UV(0.0f, 0.0f));
    m_backGround->uv.push_back(UV(0.0f, 1.0f));
    m_backGround->uv.push_back(UV(1.0f, 0.0f));
    m_backGround->uv.push_back(UV(1.0f, 1.0f));

    m_backGroundTexture = Video::instance->loadTexture("paradise", false);
    m_backGroundTexture->setFilter(Texture::Bilinear);

    loadMenu(userProfile, unlockable, current);
    Input::instance->startButtonBuffer();
    Input::instance->startKeyBuffer();
    Input::instance->startCharBuffer();
    glfwEnable(GLFW_MOUSE_CURSOR);
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

    Submenu* submenu = new Submenu(m_font, m_fontBig);

    submenu->setTitle(language->get(TEXT_MAIN_MENU), titlePos);

    submenu->addEntry(new SubmenuEntry(language->get(TEXT_START_SINGLEPLAYER), this, "startSingle"));
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_PLAYER_OPTIONS), this, "playerOptions"));
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_OPTIONS), this, "options"));
    submenu->addEntry(new QuitEntry(language->get(TEXT_QUIT_GAME), this));

    m_currentSubmenu = submenu;
    submenu->center(submenuPosition);
    m_submenus["main"] = submenu;


    // Start Game Submenu

    submenu = new Submenu(m_font, m_fontBig);

    submenu->setTitle(language->get(TEXT_START_SINGLEPLAYER), titlePos);

    submenu->addEntry(new WorldEntry(language->get(TEXT_EASY), this, 0, current));
    submenu->addEntry(new WorldEntry(language->get(TEXT_NORMAL), this, 1, current));
    if (unlockable < 1)
    {
        submenu->m_entries.back()->disable();
    }
    submenu->addEntry(new WorldEntry(language->get(TEXT_HARD), this, 2, current));
    if (unlockable < 2)
    {
        submenu->m_entries.back()->disable();
    }
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_BACK), this, "main"));

    submenu->center(submenuPosition);
    m_submenus["startSingle"] = submenu;



    // PLAYER Options Submenu
    submenu = new Submenu(m_font, m_fontBig);
    
    submenu->setTitle(language->get(TEXT_PLAYER_OPTIONS), titlePos);

    submenu->addEntry(new WritableEntry(language->get(TEXT_NAME), userProfile->m_name, submenu));

    ColorValue colorValue("color");
    submenu->addEntry(new ColorEntry(language->get(TEXT_COLOR), userProfile->m_color, colorValue));

    submenu->addEntry(new SubmenuEntry(language->get(TEXT_BACK), this, "main"));    

    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["playerOptions"] = submenu;

    // Options Submenu
    submenu = new Submenu(m_font, m_fontBig);
    
    submenu->setTitle(language->get(TEXT_OPTIONS), titlePos);

    submenu->addEntry(new SubmenuEntry(language->get(TEXT_VIDEO_OPTIONS), this, "videoOptionsEasy"));
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_AUDIO_OPTIONS), this, "audioOptions"));
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_OTHER_OPTIONS), this, "otherOptions"));

    submenu->addEntry(new SpacerEntry());
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_BACK), this, "main"));    

    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["options"] = submenu;


    // VIDEO Options NOOBS Submenu
    submenu = new Submenu(m_font, m_fontBig);
    
    submenu->setTitle(language->get(TEXT_VIDEO_OPTIONS), titlePos);

    IntPairVector resolutions = Video::instance->getModes();
    Value valueRes("resolution");
    for (size_t i = 0; i < resolutions.size(); i++)
    {
        valueRes.add(wcast<wstring>(resolutions[i].first) 
                            + L"x" + wcast<wstring>(resolutions[i].second));
    }
    submenu->addEntry(new OptionEntry(language->get(TEXT_RESOLUTION), valueRes));

    BoolValue valFS("fullscreen");
    submenu->addEntry(new OptionEntry(language->get(TEXT_FULLSCREEN), valFS));

    Value valEnvDet("environment_details");
    valEnvDet.add(language->get(TEXT_LOW));
    valEnvDet.add(language->get(TEXT_MEDIUM));
    valEnvDet.add(language->get(TEXT_HIGH));
    valEnvDet.add(language->get(TEXT_CUSTOM));
    submenu->addEntry(new OptionEntry(language->get(TEXT_ENVIRONMENT_DETAILS), valEnvDet));

    submenu->addEntry(new SubmenuEntry(language->get(TEXT_VIDEO_OPTIONS_ADVANCED), this, "videoOptions"));

    submenu->addEntry(new SpacerEntry());
    submenu->addEntry(new ApplyOptionsEntry(language->get(TEXT_SAVE), this, "videoOptionsEasy"));
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_BACK), this, "options"));    
    
    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["videoOptionsEasy"] = submenu;

    // VIDEO Options ADVANCED Submenu
    submenu = new Submenu(m_font, m_fontBig);
    
    submenu->setTitle(language->get(TEXT_VIDEO_OPTIONS_ADVANCED), titlePos);

    BoolValue valVS("vsync");
    submenu->addEntry(new OptionEntry(language->get(TEXT_VSYNC), valVS));

    Value valFSAA("fsaa_samples");
    valFSAA.add(L"0");
    valFSAA.add(L"2");
    valFSAA.add(L"4");
    valFSAA.add(L"6");
    valFSAA.add(L"8");
    submenu->addEntry(new OptionEntry(language->get(TEXT_FSAA), valFSAA));

    BoolValue valSh("use_shaders");
    submenu->addEntry(new OptionEntry(language->get(TEXT_SHADERS), valSh));

    BoolValue valShad("shadow_type");
    submenu->addEntry(new OptionEntry(language->get(TEXT_SHADOW_TYPE), valShad));

    Value valShadS("shadowmap_size");
    valShadS.add(language->get(TEXT_LOW));
    valShadS.add(language->get(TEXT_MEDIUM));
    valShadS.add(language->get(TEXT_HIGH));
    valShadS.add(language->get(TEXT_NOT_SUPPORTED));
    submenu->addEntry(new OptionEntry(language->get(TEXT_SHADOWMAP_SIZE), valShadS));

    Value valGD("grass_density");
    valGD.add(language->get(TEXT_LOW));
    valGD.add(language->get(TEXT_MEDIUM));
    valGD.add(language->get(TEXT_HIGH));
    submenu->addEntry(new OptionEntry(language->get(TEXT_GRASS_DENSITY), valGD));

    Value valTD("terrain_detail");
    valTD.add(language->get(TEXT_LOW));
    valTD.add(language->get(TEXT_MEDIUM));
    valTD.add(language->get(TEXT_HIGH));
    submenu->addEntry(new OptionEntry(language->get(TEXT_TERRAIN_DETAIL), valTD));

    BoolValue valFPS("show_fps");
    submenu->addEntry(new OptionEntry(language->get(TEXT_SHOW_FPS), valFPS));

    submenu->addEntry(new SpacerEntry());
    submenu->addEntry(new ApplyOptionsEntry(language->get(TEXT_SAVE), this, "videoOptions"));
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_BACK), this, "videoOptionsEasy"));    
    
    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["videoOptions"] = submenu;


    // AUDIO Options Submenu
    submenu = new Submenu(m_font, m_fontBig);
    
    submenu->setTitle(language->get(TEXT_AUDIO_OPTIONS), titlePos);

    BoolValue valAud("audio");
    submenu->addEntry(new OptionEntry(language->get(TEXT_AUDIO), valAud));

    submenu->addEntry(new SpacerEntry());
    submenu->addEntry(new ApplyOptionsEntry(language->get(TEXT_SAVE), this, "audioOptions"));
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_BACK), this, "options"));    
    
    submenu->center(submenuPosition);
    titleY = std::max(submenu->m_upper.y, titleY);
    m_submenus["audioOptions"] = submenu;

    
    
    // OTHER Options Submenu
    submenu = new Submenu(m_font, m_fontBig);
    
    submenu->setTitle(language->get(TEXT_OTHER_OPTIONS), titlePos);

    Value valLang("language");
    valLang.add(L"ENG");
    valLang.add(L"LAT");
    valLang.add(L"RUS");
    submenu->addEntry(new OptionEntry(language->get(TEXT_LANGUAGE), valLang));

    BoolValue valSysK("system_keys");
    submenu->addEntry(new OptionEntry(language->get(TEXT_SYSTEM_KEYS), valSysK));

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

    submenu->addEntry(new SpacerEntry());
    submenu->addEntry(new ApplyOptionsEntry(language->get(TEXT_SAVE), this, "otherOptions"));
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_BACK), this, "options"));    

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
    glfwDisable(GLFW_MOUSE_CURSOR);
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
    return m_state;
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
                    m_currentSubmenu->m_entries.back()->click(GLFW_KEY_ENTER);
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

    glBindTexture(GL_TEXTURE_2D, m_font->m_texture);
    m_currentSubmenu->render();
    m_font->end();
}
