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

typedef vector<wstring> Values;
class Submenu;

class Value
{
    friend class OptionEntry;
public:
    Value(const string& id) : m_current(0), m_id(id) {}
    void add(const wstring& string)                  { m_values.push_back(string); }
    
    wstring getCurrent() const
    {
        if (m_values.empty())
        {
            return L"";
        }
        return m_values[m_current];
    }

    void activateNext(bool forward)
    {
        if (!m_values.empty())
        {
            if (forward)
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
            else
            {
                if (m_current == 0)
                {
                    m_current = m_values.size() - 1;
                }
                else
                {
                    m_current--;
                }
            }

        }
    }

    int getMaxWidth(const Font* font) const
    {
        int maxW = 0;
        for each_const(Values, m_values, value)
        {
            int w = font->getWidth(*value);
            if (w > maxW)
            {
                maxW = w;
            }
        }
        return maxW;
    }


protected:
    Values m_values;
    size_t m_current;
    string m_id;
};


class BoolValue : public Value
{
public:
    BoolValue(const string& id) : Value(id)
    {
        Value::add(Language::instance->get(TEXT_FALSE));
        Value::add(Language::instance->get(TEXT_TRUE));
    }

private:
    void add(const wstring& string);
};

class Entry
{
public: 
    wstring     m_string;
    Vector      m_lowerLeft;
    Vector      m_upperRight;

    Entry(const wstring& stringIn) : m_string(stringIn) {}
    virtual ~Entry() {}

    virtual void click(int button) = 0;
    virtual wstring getString() const                    { return m_string; }
    virtual string getValueID() const                    { return "";       }
    virtual wstring getValue() const                     { return L"";      }
    virtual int getCurrentValueIdx() const               { return -1;       }
    virtual bool isEnabled() const                       { return true;     }
    virtual void reset()                                 {}

    bool isMouseOver(const Vector& mousePos) const       { return isPointInRectangle(mousePos, m_lowerLeft, m_upperRight); }

    void calculateBounds(const Vector& position, const Font* font)
    {
        m_lowerLeft = Vector(position.x - font->getWidth(getString())/2.0f, 0, position.y);
        m_upperRight = Vector(position.x + font->getWidth(getString())/2.0f, 0, position.y + font->getHeight());
    }
    void setXBound(float minX, float maxX)
    {
        m_lowerLeft.x = minX;
        m_upperRight.x = maxX;
    }

    virtual void render(const Font* font) const          { font->render(getString(), Font::Align_Center); }

    virtual int getMaxLeftWidth(const Font* font) const
    {
        return font->getWidth(m_string)/2;
    }

    virtual int getMaxRightWidth(const Font* font) const
    {
        return font->getWidth(m_string)/2;
    }

};

class WritableEntry : public Entry
{
public: 
    WritableEntry(const wstring& label, string& binding, const Submenu* ownerSubmenu) :
      Entry(label), m_binding(binding), m_ownerSubmenu(ownerSubmenu) {}
    void render(const Font* font) const;
    void click(int button);

private:
    const Submenu* m_ownerSubmenu;
    string&        m_binding;
};

void WritableEntry::render(const Font* font) const
{
    wstring stringToRender = m_string + L": " + wcast<wstring>(m_binding);
    if (m_ownerSubmenu->m_entries[m_ownerSubmenu->m_activeEntry] == this)
    {
        stringToRender.push_back('_');
    }
    font->render(stringToRender, Font::Align_Left);
}

void WritableEntry::click(int button) 
{ 
    if ((button > 31) && (button < 127) && ((m_string.size() + m_binding.size()) < 15))
    {
        m_binding.push_back(button);
    }
    else if (button == GLFW_KEY_BACKSPACE)
    {
        if (m_binding.size() > 0)
        {
             m_binding.resize(m_binding.size() - 1);
        }
    }
}

class OptionEntry : public Entry
{
public: 
    OptionEntry(const wstring& stringIn, const Value& value) :
        Entry(stringIn), m_value(value), m_enabled(true) {}

    void click(int button);
    wstring getString() const                      { return m_string +  L":  " + m_value.getCurrent(); }
    string getValueID() const                      { return m_value.m_id; }
    wstring getValue() const                       { return m_value.getCurrent(); }
    int getCurrentValueIdx() const                 { return static_cast<int>(m_value.m_current); }
    bool isEnabled() const                         { return m_enabled; }
    void reset();
    
    bool isMouseOver(const Vector& mousePos) const { return m_enabled && Entry::isMouseOver(mousePos); }
    
    void render(const Font* font) const;
    
    virtual int getMaxLeftWidth(const Font* font) const
    {
        return font->getWidth(m_string) + font->getWidth(L":");
    }

    virtual int getMaxRightWidth(const Font* font) const
    {
        return  font->getWidth(L"  ") +  m_value.getMaxWidth(font);
    }

private:
    Value m_value;
    bool m_enabled;
};

void OptionEntry::click(int button)
{
    if ((button == GLFW_MOUSE_BUTTON_LEFT)
        || (button == GLFW_MOUSE_BUTTON_RIGHT)
        || (button == GLFW_KEY_LEFT)
        || (button == GLFW_KEY_RIGHT)
        || (button == GLFW_KEY_ENTER))
    {
        if (m_enabled)
        {
            m_value.activateNext((button == GLFW_MOUSE_BUTTON_LEFT) 
                                  || (button == GLFW_KEY_RIGHT)
                                  || (button == GLFW_KEY_ENTER));
        }
    }
}

class GameEntry : public Entry
{
public: 
    GameEntry(const wstring& stringIn, Menu* menu, State::Type stateToSwitchTo) :
      Entry(stringIn), m_menu(menu), m_stateToSwitchTo(stateToSwitchTo) {}
    
    void click(int button);

private:
    Menu* m_menu;
    State::Type m_stateToSwitchTo;

};

void GameEntry::click(int button) 
{ 
    if ((button == GLFW_MOUSE_BUTTON_LEFT) || (button == GLFW_KEY_ENTER) || (button == GLFW_KEY_KP_ENTER))
    {
        m_menu->setState(m_stateToSwitchTo);
    }
}

class SubmenuEntry : public Entry
{
public: 
    SubmenuEntry(const wstring& stringIn, Menu* menu, const string&  submenuToSwitchTo) :
        Entry(stringIn), m_menu(menu), m_submenuToSwitchTo(submenuToSwitchTo) {}

    void click(int button); 

protected:
    Menu*  m_menu;
    string m_submenuToSwitchTo;
};

void SubmenuEntry::click(int button) 
{ 
    if ((button == GLFW_MOUSE_BUTTON_LEFT) || (button == GLFW_KEY_ENTER) || (button == GLFW_KEY_KP_ENTER))
    {
        m_menu->setSubmenu(m_submenuToSwitchTo); 
    }
}

class ApplyOptionsEntry : public SubmenuEntry
{
public: 
    ApplyOptionsEntry(const wstring& stringIn, Menu* menu, const string&  submenuToSwitchTo) :
      SubmenuEntry(stringIn, menu, submenuToSwitchTo) {}
    void click(int button);
};

class SpacerEntry : public Entry
{
public: 
    SpacerEntry() : Entry(L"") {}
    void click(int button)     {}
    wstring getString() const  { return L""; }
    bool isEnabled() const     { return false; }
};

void OptionEntry::reset()
{
    // Sync option values to values in Config object

    if (m_value.m_id == "environment_details")
    {
        int shMaps = Config::instance->m_video.shadowmap_size;
        int grDens = Config::instance->m_video.grass_density;
        int terDet = Config::instance->m_video.terrain_detail;
        
        for (int i = 0; i < 4; i++)
        {
            if (((!Video::instance->m_haveShadows) || (shMaps == i)) && (grDens == i) && (terDet == i))
            {
                m_value.m_current = i;
                break;
            }
            m_value.m_current = i;
        }
    }
    else if (m_value.m_id == "resolution")
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
            m_value.m_current = 3; //not supported
        }
        else
        {
            m_value.m_current = Config::instance->m_video.shadowmap_size;
        }
    }
    else if (m_value.m_id == "show_fps")
    {
        m_value.m_current = Config::instance->m_video.show_fps ? 1 : 0;
    }
    else if (m_value.m_id == "grass_density")
    {
        m_value.m_current = Config::instance->m_video.grass_density;
    }
    else if (m_value.m_id == "terrain_detail")
    {
        m_value.m_current = Config::instance->m_video.terrain_detail;
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

void OptionEntry::render(const Font* font) const
{
    glPushMatrix();
    font->render(m_string + L":", Font::Align_Right);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(static_cast<float>(font->getWidth(L"  ")), 0.0f, 0.0f);
    font->render(getValue(), Font::Align_Left);
    glPopMatrix();
}

void ApplyOptionsEntry::click(int button)
{
    if ((button != GLFW_MOUSE_BUTTON_LEFT) && (button != GLFW_KEY_ENTER) && (button != GLFW_KEY_KP_ENTER))
    {
        return;
    }

    //save config settings
    for each_const(Entries, m_menu->m_currentSubmenu->m_entries, iter)
    {
        string id = (*iter)->getValueID();

        if (id == "environment_details")
        {
            if ((*iter)->getCurrentValueIdx() != 3)
            {
                if (Video::instance->m_haveShadows)
                {
                    Config::instance->m_video.shadowmap_size = (*iter)->getCurrentValueIdx();
                }
                Config::instance->m_video.grass_density = (*iter)->getCurrentValueIdx();
                Config::instance->m_video.terrain_detail = (*iter)->getCurrentValueIdx();
            }
        }

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
            Config::instance->m_video.samples = 2 * (*iter)->getCurrentValueIdx();
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
            if (Video::instance->m_haveShadows)
            {
                Config::instance->m_video.shadowmap_size = (*iter)->getCurrentValueIdx();
            }
        }
        else if (id == "show_fps")
        {
            Config::instance->m_video.show_fps = (*iter)->getCurrentValueIdx()==1;
        }
        else if (id == "grass_density")
        {
            Config::instance->m_video.grass_density = (*iter)->getCurrentValueIdx();
        }
        else if (id == "terrain_detail")
        {
            Config::instance->m_video.terrain_detail = (*iter)->getCurrentValueIdx();
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
    g_optionsEntry = m_submenuToSwitchTo;
}

void Submenu::control(int key)
{
    int b;
    do
    {
        b = Input::instance->popButton();
    }
    while (Input::instance->popButton() != -1);
    
    //get mouse position
    const Mouse& mouse = Input::instance->mouse();
    int videoHeight = Video::instance->getResolution().second;
    Vector mousePos = Vector(static_cast<float>(mouse.x), 0, static_cast<float>(videoHeight - mouse.y));

    //adjust active entry depending on up/down keys
    if ((key == GLFW_KEY_DOWN) || (key == GLFW_KEY_UP))
    {
        activateNextEntry(key == GLFW_KEY_DOWN);
    }

    //adjust active entry depending on mouse position
    for (size_t i = 0; i < m_entries.size(); i++)
    {
        Entry* currentEntry = m_entries[i];
        if ( (m_previousMousePos != mousePos)
            && currentEntry->isMouseOver(mousePos)
            && (currentEntry->isEnabled()))
        {
            m_activeEntry = i;
            break;
        }
    }

    Entry* currentEntry = m_entries[m_activeEntry];

    
    if (b != -1)
    {
        if (currentEntry->isMouseOver(mousePos))
        {
            currentEntry->click(b);
        }
    }
    else if (key != -1)
    {
        currentEntry->click(key);
    }

    m_previousMousePos = mousePos;
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
    m_height += m_font->getHeight() + 2;
}

void Submenu::center(const Vector& centerPos)
{
    m_centerPos = centerPos;
    Vector upperPos = centerPos;

    upperPos.y += m_height / 2 - m_font->getHeight() / 2;

    m_upper.y = upperPos.y + m_font->getHeight();

    int maxX = 0;
        
    for (size_t i = 0; i < m_entries.size(); i++)
    {      
        Entry* entry = m_entries[i];
        entry->calculateBounds(upperPos, m_font);
        upperPos.y -= m_font->getHeight() - 2;
        
        int l = entry->getMaxLeftWidth(m_font);
        int r = entry->getMaxRightWidth(m_font);
        if (l > maxX)
        {
            maxX = l;
        }
        if (r > maxX)
        {
            maxX = r;
        }
    }

    for (size_t i=0; i<m_entries.size(); i++)
    {
        m_entries[i]->setXBound(centerPos.x - maxX, centerPos.x + maxX);
    }

    m_upper.x = centerPos.x + maxX; //std::max(maxR, maxL);
    m_lower.x = centerPos.x - maxX; //std::max(maxR, maxL);
    m_lower.y = upperPos.y + m_font->getHeight() - 2;
}

void Submenu::setTitle(const wstring& title, const Vector& position)
{
    m_title = title;
    m_titlePos = position;
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
    glDisable(GL_TEXTURE_2D);
    Video::instance->renderRoundRect(m_lower, m_upper, static_cast<float>(m_font->getHeight()/2));
    glEnable(GL_TEXTURE_2D);

    Vector upperPos = m_centerPos;
    upperPos.y += m_height / 2 - m_font->getHeight() / 2;

    for (size_t i = 0; i < m_entries.size(); i++)
    {
        Entry* entry = m_entries[i];

        glPushMatrix();
        glTranslatef(upperPos.x, upperPos.y, upperPos.z);
        if (m_activeEntry == i)
        {
            glColor3fv(Vector(1, 1, 0).v);              // active entry
        }
        else
        {
            if (entry->isEnabled())
            {
                glColor3fv(Vector::One.v);              // normal entry
            }
            else
            {
                glColor3fv(Vector(0.5f, 0.5f, 0.5f).v); // disabled entry
            }
        }
        entry->render(m_font);
        glPopMatrix();
        upperPos.y -= m_font->getHeight() - 2;
    }

    if (!m_title.empty())
    {
        // TODO: move somewhere else
        m_fontBig->begin();
        glPushMatrix();
        glTranslatef(m_titlePos.x, m_titlePos.y, 0);
        glColor3fv(Vector(0, 0.7f, 0).v);
        m_fontBig->render(m_title, Font::Align_Center);
        glPopMatrix();
        m_fontBig->end();
    }
}

Menu::Menu(string& userName) : m_font(Font::get("Arial_32pt_bold")), m_fontBig(Font::get("Arial_72pt_bold")), m_state(State::Current)
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

    loadMenu(userName);
    Input::instance->startButtonBuffer();
    Input::instance->startKeyBuffer();
    glfwEnable(GLFW_MOUSE_CURSOR);
}

void Menu::loadMenu(string& userName)
{
    Language* language = Language::instance;

    int resX = Video::instance->getResolution().first;
    int resY = Video::instance->getResolution().second;

    Vector submenuPosition = Vector(static_cast<float>(resX) / 2,
                                    static_cast<float>(resY) / 4 * 3 / 2, 
                                    0);

    // Main Submenu

    Submenu* submenu = new Submenu(m_font, m_fontBig);

    submenu->addEntry(new GameEntry(language->get(TEXT_START_GAME), this, State::World));
    
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_OPTIONS), this, "options"));
    submenu->addEntry(new WritableEntry(language->get(TEXT_TRUE), userName, submenu));

    submenu->addEntry(new GameEntry(language->get(TEXT_QUIT_GAME), this, State::Quit));

    m_currentSubmenu = submenu;
    submenu->center(submenuPosition);
    m_submenus["main"] = submenu;


    // Options Submenu
    submenu = new Submenu(m_font, m_fontBig);
    
    Vector titlePos = Vector(static_cast<float>(resX) / 2, 0, 0);
    float titleY = 0.0f;

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
    do
    {
        key = Input::instance->popKey();
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
    while (Input::instance->popKey() != -1);
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
