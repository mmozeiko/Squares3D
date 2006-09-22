#include <GL/glfw.h>

#include "menu.h"
#include "game.h"
#include "world.h"
#include "font.h"
#include "video.h"
#include "texture.h"
#include "geometry.h"
#include "input.h"
#include "language.h"
#include "config.h"

void Value::addAnother(const wstring& string)
{
    m_values.push_back(string);
}

Value::Value(const string& id) :
    m_current(0),
    m_id(id)
{
}

wstring Value::getCurrent() const
{
    wstring returnVal;
    if (!m_values.empty())
    {
        returnVal = m_values[m_current];
    }
    return returnVal;
}

void Value::activateNext()
{
    if (!m_values.empty())
    {
        if (m_current == (m_values.size() - 1))
        {
            m_current = 0;
        }
        else
        {
            m_current++;
        }
    }
}

BoolValue::BoolValue(const string& id) :
    Value(id)
{
    m_values.push_back(Language::instance->get(TEXT_FALSE));
    m_values.push_back(Language::instance->get(TEXT_TRUE));
}

void BoolValue::addAnother(const wstring& string)
{
    assert(false);
}


Entry::Entry(const wstring& stringIn, const Font* font) :
    m_font(font),
    m_string(stringIn)
{
}

void Entry::calculateBounds(const Vector& position)
{
    m_lowerLeft = Vector(position.x - m_font->getWidth(getString()), 0, position.y);
    m_upperRight = Vector(position.x + m_font->getWidth(getString()), 
                          0,
                          position.y + m_font->getHeight());
}


string Entry::getValueID() const
{
    return "";
}

size_t Entry::getCurrentValueIdx() const
{
    return -1;
}

wstring Entry::getValue() const
{
    return L"";
}

bool Entry::isEnabled() const
{
    return true;
}

bool Entry::isMouseOver(const Vector& mousePos) const
{
    return isPointInRectangle(mousePos, m_lowerLeft, m_upperRight);
}

OptionEntry::OptionEntry(const wstring& stringIn, const Value& value, const Font* font) : 
    Entry(stringIn, font),
    m_value(value),
    m_enabled(true)
{
}

bool OptionEntry::isEnabled() const
{
    return m_enabled;
}

wstring OptionEntry::getString() const
{
    return m_string +  L": " + m_value.getCurrent();
}

wstring OptionEntry::getValue() const
{
    return m_value.getCurrent();
}

bool OptionEntry::isMouseOver(const Vector& mousePos) const
{
    if (m_enabled)
    {
        return Entry::isMouseOver(mousePos);
    }
    else
    {
        return false;
    }
}

string OptionEntry::getValueID() const
{
    return m_value.m_id;
}

size_t OptionEntry::getCurrentValueIdx() const
{
    return m_value.m_current;
}

void OptionEntry::click()
{
    if (m_enabled)
    {
        m_value.activateNext();
    }
}

void OptionEntry::reset()
{
    // Sync option values to values in Config object

    if (m_value.m_id == "resolution")
    {
        const IntPairVector resolutions = Video::instance->getModes();
        for (size_t i = 0; i < resolutions.size(); i++)
        {
            if (Video::instance->getResolution() == resolutions[i])
            {
                m_value.m_current = i;
                break;
            }
        }
    }
    else if (m_value.m_id == "fullscreen")
    {
        m_value.m_current = Config::instance->m_video.fullscreen ? 1 : 0;
    }
    else if (m_value.m_id == "vsync")
    {
        m_value.m_current = Config::instance->m_video.vsync ? 1 : 0;
    }
    else if (m_value.m_id == "fsaa_samples")
    {
        m_value.m_current = Config::instance->m_video.samples / 2;
    }
    else if (m_value.m_id == "use_shaders")
    {
        if (!Video::instance->m_haveShaders)
        {
            m_enabled = false;
        }
        m_value.m_current = Config::instance->m_video.use_shaders ? 1 : 0;
    }
    else if (m_value.m_id == "shadow_type")
    {
        if (!Video::instance->m_haveShadows)
        {
            m_enabled = false;
        }
        m_value.m_current = Config::instance->m_video.shadow_type ? 1 : 0;
    }
    else if (m_value.m_id == "shadowmap_size")
    {
        if (!Video::instance->m_haveShadows)
        {
            m_enabled = false;
        }
        m_value.m_current = Config::instance->m_video.shadowmap_size / 1024;
    }
    else if (m_value.m_id == "show_fps")
    {
        m_value.m_current = Config::instance->m_video.show_fps ? 1 : 0;
    }
    else if (m_value.m_id == "audio")
    {
        m_value.m_current = Config::instance->m_audio.enabled ? 1 : 0;
    }
    else if (m_value.m_id == "language")
    {
        StringVector languages = Language::instance->getAvailable();
        for (size_t i = 0; i < languages.size(); i++)
        {
            if (languages[i] == Config::instance->m_misc.language)
            {
                m_value.m_current = i;
                break;
            }
        }
    }

}

GameEntry::GameEntry(const wstring& stringIn, 
                     Menu* menu, 
                     State::Type stateToSwitchTo, 
                     const Font* font) : 
    Entry(stringIn, font),
    m_menu(menu),
    m_stateToSwitchTo(stateToSwitchTo)
{
}

wstring GameEntry::getString() const
{
    return m_string;
}    

void GameEntry::click()
{
    m_menu->setState(m_stateToSwitchTo);
}

SubmenuEntry::SubmenuEntry(const wstring& stringIn, 
                           Menu*          menu, 
                           const string&  submenuToSwitchTo, 
                           const   Font*  font) : 
    Entry(stringIn, font),
    m_menu(menu),
    m_submenuToSwitchTo(submenuToSwitchTo)
{
}

wstring SubmenuEntry::getString() const
{
    return m_string;
}    

void SubmenuEntry::click()
{
    m_menu->setSubmenu(m_submenuToSwitchTo);
}

ApplyOptionsEntry::ApplyOptionsEntry(const wstring& stringIn, 
                                     Menu*          menu, 
                                     const string&  submenuToSwitchTo, 
                                     const   Font*  font) :
    SubmenuEntry(stringIn, menu, submenuToSwitchTo, font)
{
}

void ApplyOptionsEntry::click()
{
    //save config settings
    for each_const(Entries, m_menu->m_currentSubmenu->m_entries, iter)
    {
        string id = (*iter)->getValueID();

        if (id == "resolution")
        {
            const IntPair mode = Video::instance->getModes()[(*iter)->getCurrentValueIdx()];
            Config::instance->m_video.width = mode.first;
            Config::instance->m_video.height = mode.second;
        }
        else if (id == "fullscreen")
        {
            Config::instance->m_video.fullscreen = (*iter)->getCurrentValueIdx()==1;
        }
        else if (id == "vsync")
        {
            Config::instance->m_video.vsync = (*iter)->getCurrentValueIdx()==1;
        }
        else if (id == "fsaa_samples")
        {
            size_t idx = (*iter)->getCurrentValueIdx();
            Config::instance->m_video.samples = 2 * static_cast<int>(idx);
        }
        else if (id == "use_shaders")
        {
            Config::instance->m_video.use_shaders = (*iter)->getCurrentValueIdx()==1;
        }
        else if (id == "shadow_type")
        {
            Config::instance->m_video.shadow_type = (*iter)->getCurrentValueIdx()==1;
        }
        else if (id == "shadowmap_size")
        {
            size_t idx = (*iter)->getCurrentValueIdx();
            Config::instance->m_video.shadowmap_size = 512 << idx;
        }
        else if (id == "show_fps")
        {
            Config::instance->m_video.show_fps = (*iter)->getCurrentValueIdx()==1;
        }
        else if (id == "audio")
        {
            Config::instance->m_audio.enabled = (*iter)->getCurrentValueIdx()==1;
        }
        else if (id == "language")
        {
            StringVector languages = Language::instance->getAvailable();
            Config::instance->m_misc.language = languages[(*iter)->getCurrentValueIdx()];
        }
    }
    
    m_menu->setState(State::Quit);
    g_needsToReload = true;
}

void Submenu::control(const int key)
{
    bool left_button = (Input::instance->popButton() == GLFW_MOUSE_BUTTON_LEFT);
    
    bool enter_key = (key == GLFW_KEY_ENTER);
    bool down_key = (key == GLFW_KEY_DOWN);
    bool up_key = (key == GLFW_KEY_UP);


    //get mouse position
    const Mouse& mouse = Input::instance->mouse();
    int videoHeight = Video::instance->getResolution().second;
    Vector mousePos = Vector(static_cast<float>(mouse.x), 0, static_cast<float>(videoHeight - mouse.y));

    //adjust active entry depending on up/down keys
    if (down_key || up_key)
    {
        activateNextEntry(down_key);
    }

    //adjust active entry depending on mouse position
    for (size_t i = 0; i < m_entries.size(); i++)
    {
        Entry* currentEntry = m_entries[i];
        if (currentEntry->isEnabled() 
            && (m_previousMousePos != mousePos)
            && currentEntry->isMouseOver(mousePos))
        {
            m_activeEntry = i;
            break;
        }
    }

    Entry* currentEntry = m_entries[m_activeEntry];

    if (enter_key || (left_button && currentEntry->isMouseOver(mousePos)))
    {
        currentEntry->click();
    }

    m_previousMousePos = mousePos;
}

Submenu::Submenu() : 
    m_activeEntry(0),
    m_title(L""),
    m_titleFont(NULL),
    m_height(0)
{
}

Submenu::~Submenu()
{
    for each_const(Entries, m_entries, iter)
    {
        delete *iter;
    }
}

void Submenu::addEntry(Entry* entry)
{
    m_entries.push_back(entry);
    m_height += entry->m_font->getHeight() + 2;

}

void Submenu::center(const Vector& centerPos)
{
    m_centerPos = centerPos;
    Vector upperPos = centerPos;
    upperPos.y += m_height / 2 - m_entries[0]->m_font->getHeight() / 2;
    for (size_t i = 0; i < m_entries.size(); i++)
    {
        
        Entry* currentEntry = m_entries[i];
        currentEntry->calculateBounds(upperPos);
        upperPos.y -= m_entries[i]->m_font->getHeight() - 2;
    }
}

void Submenu::setTitle(const wstring& title, const Vector& position, const Font* font)
{
    m_title = title;
    m_titlePos = position;
    m_titleFont = font;
}

void Submenu::activateNextEntry(bool moveDown)
{
    //should be used just for key input (up/down used)
    if (!m_entries.empty())
    {
        if (moveDown && (m_activeEntry == (m_entries.size() - 1)))
        {
            m_activeEntry = 0;
        }
        else if (!moveDown && (m_activeEntry == 0))
        {
            m_activeEntry = m_entries.size() - 1;
        }
        else
        {
            moveDown ? m_activeEntry++ : m_activeEntry--;
        }
        if (!m_entries[m_activeEntry]->isEnabled())
        {
            activateNextEntry(moveDown);
        }
    }
}

void Submenu::render() const
{
    if (!m_title.empty())
    {
        glPushMatrix();
        glTranslatef(m_titlePos.x, m_titlePos.y, 0);
        glScalef(2.5f, 2.5f, 2.5f);
        glColor3fv(Vector(0, 0.7f, 0).v);
        m_titleFont->render(m_title, Font::Align_Center);
        glPopMatrix();
    }

    Vector upperPos = m_centerPos;
    upperPos.y += m_height / 2 - m_entries[0]->m_font->getHeight() / 2;

    for (size_t i = 0; i < m_entries.size(); i++)
    {
        Entry* currentEntry = m_entries[i];

        glPushMatrix();
        glTranslatef(upperPos.x, upperPos.y, upperPos.z);
        if (m_activeEntry == i)
        {
            glColor3fv(Vector::One.v);
        }
        else
        {
            if (currentEntry->isEnabled())
            {
                glColor3fv(Vector::Zero.v);
            }
            else
            {
                glColor3fv(Vector(0.5f, 0.5f, 0.5f).v);
            }
        }
        currentEntry->m_font->render(currentEntry->getString(), Font::Align_Center);
        glPopMatrix();   
        upperPos.y -= m_entries[i]->m_font->getHeight() - 2;
    }
}

Menu::Menu() :
    m_font(Font::get("Arial_32pt_bold")),
    m_state(State::Current)
{
    float resX = static_cast<float>(Video::instance->getResolution().first);
    float resY = static_cast<float>(Video::instance->getResolution().second);

    m_backGround = new Face();
    m_backGround->vertexes.push_back(Vector::Zero);
    m_backGround->vertexes.push_back(Vector(0, resY, 0));
    m_backGround->vertexes.push_back(Vector(resX, 0, 0));
    m_backGround->vertexes.push_back(Vector(resX, resY, 0));

    m_backGround->uv.push_back(UV(0, 0));
    m_backGround->uv.push_back(UV(0, 1));
    m_backGround->uv.push_back(UV(1, 0));
    m_backGround->uv.push_back(UV(1, 1));

    m_backGroundTexture = Video::instance->loadTexture("paradise");

    loadMenu();
    Input::instance->startButtonBuffer();
    Input::instance->startKeyBuffer();
}

void Menu::loadMenu()
{
    Language* language = Language::instance;

    int resX = Video::instance->getResolution().first;
    int resY = Video::instance->getResolution().second;

    Vector submenuPosition = Vector(static_cast<float>(resX) / 2,
                                    static_cast<float>(resY) / 4 * 3 / 2, 
                                    0);

    // Main Submenu

    Submenu* submenu = new Submenu();

    submenu->addEntry(new GameEntry(language->get(TEXT_START_GAME), this, State::World, m_font));
    
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_OPTIONS), this, "options", m_font));

    submenu->addEntry(new GameEntry(language->get(TEXT_QUIT_GAME), this, State::Quit, m_font));

    m_currentSubmenu = submenu;
    submenu->center(submenuPosition);
    m_submenus["main"] = submenu;

    // Options Submenu
    submenu = new Submenu();
    
    Vector titlePos = Vector(static_cast<float>(resX) / 2,
                             resY - static_cast<float>(resY) / 6, 
                             0);
    submenu->setTitle(language->get(TEXT_OPTIONS), titlePos, m_font);

    submenu->addEntry(new SubmenuEntry(language->get(TEXT_VIDEO_OPTIONS), this, "videoOptions", m_font));
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_AUDIO_OPTIONS), this, "audioOptions", m_font));

    Value valLang("language");
    valLang.addAnother(L"ENG");
    valLang.addAnother(L"LAT");
    valLang.addAnother(L"RUS");
    submenu->addEntry(new OptionEntry(language->get(TEXT_LANGUAGE), valLang, m_font));

    submenu->addEntry(new ApplyOptionsEntry(language->get(TEXT_SAVE), this, "options", m_font));

    submenu->addEntry(new SubmenuEntry(language->get(TEXT_BACK), this, "main", m_font));    

    submenu->center(submenuPosition);
    m_submenus["options"] = submenu;


    // VIDEO Options Submenu
    submenu = new Submenu();
    
    submenu->setTitle(language->get(TEXT_VIDEO_OPTIONS), titlePos, m_font);

    IntPairVector resolutions = Video::instance->getModes();
    Value valueRes("resolution");
    for (size_t i = 0; i < resolutions.size(); i++)
    {
        valueRes.addAnother(wcast<wstring>(resolutions[i].first) + L"x" + wcast<wstring>(resolutions[i].second));
    }
    submenu->addEntry(new OptionEntry(language->get(TEXT_RESOLUTION), valueRes, m_font));

    BoolValue valFS("fullscreen");
    submenu->addEntry(new OptionEntry(language->get(TEXT_FULLSCREEN), valFS, m_font));

    BoolValue valVS("vsync");
    submenu->addEntry(new OptionEntry(language->get(TEXT_VSYNC), valVS, m_font));

    Value valFSAA("fsaa_samples");
    valFSAA.addAnother(L"0");
    valFSAA.addAnother(L"2");
    valFSAA.addAnother(L"4");
    valFSAA.addAnother(L"6");
    valFSAA.addAnother(L"8");
    submenu->addEntry(new OptionEntry(language->get(TEXT_FSAA), valFSAA, m_font));

    BoolValue valSh("use_shaders");
    submenu->addEntry(new OptionEntry(language->get(TEXT_SHADERS), valSh, m_font));

    BoolValue valShad("shadow_type");
    submenu->addEntry(new OptionEntry(language->get(TEXT_SHADOWTYPE), valShad, m_font));

    Value valShadS("shadowmap_size");
    valShadS.addAnother(L"512");
    valShadS.addAnother(L"1024");
    valShadS.addAnother(L"2048");
    submenu->addEntry(new OptionEntry(language->get(TEXT_SHADOWMAPSIZE), valShadS, m_font));

    BoolValue valFPS("show_fps");
    submenu->addEntry(new OptionEntry(language->get(TEXT_SHOWFPS), valFPS, m_font));

    submenu->addEntry(new ApplyOptionsEntry(language->get(TEXT_SAVE), this, "videoOptions", m_font));

    submenu->addEntry(new SubmenuEntry(language->get(TEXT_BACK), this, "options", m_font));    
    
    submenu->center(submenuPosition);
    m_submenus["videoOptions"] = submenu;


    // AUDIO Options Submenu
    submenu = new Submenu();
    
    submenu->setTitle(language->get(TEXT_AUDIO_OPTIONS), titlePos, m_font);

    BoolValue valAud("audio");
    submenu->addEntry(new OptionEntry(language->get(TEXT_AUDIO), valAud, m_font));

    submenu->addEntry(new ApplyOptionsEntry(language->get(TEXT_SAVE), this, "audioOptions", m_font));

    submenu->addEntry(new SubmenuEntry(language->get(TEXT_BACK), this, "options", m_font));    
    
    submenu->center(submenuPosition);
    m_submenus["audioOptions"] = submenu;

}

Menu::~Menu()
{
    Input::instance->endKeyBuffer();
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
    if (submenuToSwitchTo == "options"
        || submenuToSwitchTo == "audioOptions"
        || submenuToSwitchTo == "videoOptions")
    {
        Entries& entries = m_submenus[submenuToSwitchTo]->m_entries;
        for each_(Entries, entries, iter)
        {
            (*iter)->reset();
        }
    }
}

void Menu::control()
{
    //todo remove later
    int key = Input::instance->popKey();
    if (key == GLFW_KEY_ESC)
    {
        m_state = State::Quit;
    }
    m_currentSubmenu->control(key);
}

void Menu::update(float delta)
{
}

void Menu::updateStep(float delta)
{
}

void Menu::prepare()
{
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
