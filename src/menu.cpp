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
    virtual size_t getCurrentValueIdx() const            { return -1;       }
    virtual bool isEnabled() const                       { return true;     }
    virtual void reset()                                 {}

    bool isMouseOver(const Vector& mousePos) const       { return isPointInRectangle(mousePos, m_lowerLeft, m_upperRight); }

    void calculateBounds(const Vector& position, const Font* font)
    {
        m_lowerLeft = Vector(position.x - font->getWidth(getString())/2.0f, 0, position.y);
        m_upperRight = Vector(position.x + font->getWidth(getString())/2.0f, 0, position.y + font->getHeight());
    }

    virtual void render(const Font* font) const          { font->render(getString(), Font::Align_Center); }
};


class OptionEntry : public Entry
{
public: 
    OptionEntry(const wstring& stringIn, const Value& value) :
        Entry(stringIn), m_value(value), m_enabled(true) {}

    void click(int button);
    wstring getString() const                      { return m_string +  L": " + m_value.getCurrent(); }
    string getValueID() const                      { return m_value.m_id; }
    wstring getValue() const                       { return m_value.getCurrent(); }
    size_t getCurrentValueIdx() const              { return m_value.m_current; }
    bool isEnabled() const                         { return m_enabled; }
    void reset();
    
    bool isMouseOver(const Vector& mousePos) const { return m_enabled && Entry::isMouseOver(mousePos); }
    
    void render(const Font* font) const;

private:
    Value m_value;
    bool m_enabled;
};

void OptionEntry::click(int button)
{
    if ((button == GLFW_MOUSE_BUTTON_LEFT)
        || (button == GLFW_MOUSE_BUTTON_RIGHT)
        || (button == GLFW_KEY_LEFT)
        || (button == GLFW_KEY_RIGHT))
    {
        if (m_enabled)
        {
            m_value.activateNext((button == GLFW_MOUSE_BUTTON_RIGHT) || (button == GLFW_KEY_RIGHT));
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
    if ((button == GLFW_MOUSE_BUTTON_LEFT) || (button == GLFW_KEY_ENTER))
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
    if ((button == GLFW_MOUSE_BUTTON_LEFT) || (button == GLFW_KEY_ENTER))
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
    void click(int button)    {}
    wstring getString() const  { return L""; }
    bool isEnabled() const     { return false; }
};


typedef vector<Entry*> Entries;

class Submenu
{
public:

    Entries m_entries;
    size_t  m_activeEntry;
    float   m_height;
    Vector  m_centerPos;

    Submenu(const Font* font) : m_activeEntry(0), m_title(L""), m_font(font), m_height(0) {}
    ~Submenu();

    void addEntry(Entry* entry);
    void center(const Vector& centerPos);
    void render() const;
    void control(int key);
    void setTitle(const wstring& title, const Vector& position);
    void activateNextEntry(bool moveDown);

private:
    wstring m_title;
    Vector m_titlePos;
    const Font* m_font;

    Vector m_previousMousePos;
};



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
    if ((button != GLFW_MOUSE_BUTTON_LEFT) && (button != GLFW_KEY_ENTER))
    {
        return;
    }

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
        if (currentEntry->isEnabled() 
            && (m_previousMousePos != mousePos)
            && currentEntry->isMouseOver(mousePos))
        {
            m_activeEntry = i;
            break;
        }
    }

    Entry* currentEntry = m_entries[m_activeEntry];

    if (b != -1)
    {
        currentEntry->click(b);
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
        
    for (size_t i = 0; i < m_entries.size(); i++)
    {      
        Entry* entry = m_entries[i];
        entry->calculateBounds(upperPos, m_font);
        upperPos.y -= m_font->getHeight() - 2;
    }
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
    if (!m_title.empty())
    {
        glPushMatrix();
        glTranslatef(m_titlePos.x, m_titlePos.y, 0);
        glScalef(2.5f, 2.5f, 2.5f);
        glColor3fv(Vector(0, 0.7f, 0).v);
        m_font->render(m_title, Font::Align_Center);
        glPopMatrix();
    }

    Vector upperPos = m_centerPos;
    upperPos.y += m_height / 2 - m_font->getHeight() / 2;

    for (size_t i = 0; i < m_entries.size(); i++)
    {
        Entry* entry = m_entries[i];

        glPushMatrix();
        glTranslatef(upperPos.x, upperPos.y, upperPos.z);
        if (m_activeEntry == i)
        {
            glColor3fv(Vector::One.v);
        }
        else
        {
            if (entry->isEnabled())
            {
                glColor3fv(Vector::Zero.v);
            }
            else
            {
                glColor3fv(Vector(0.5f, 0.5f, 0.5f).v);
            }
        }
        entry->render(m_font);
        glPopMatrix();
        upperPos.y -= m_font->getHeight() - 2;
    }
}

Menu::Menu() : m_font(Font::get("Arial_32pt_bold")), m_state(State::Current)
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

    Submenu* submenu = new Submenu(m_font);

    submenu->addEntry(new GameEntry(language->get(TEXT_START_GAME), this, State::World));
    
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_OPTIONS), this, "options"));

    submenu->addEntry(new GameEntry(language->get(TEXT_QUIT_GAME), this, State::Quit));

    m_currentSubmenu = submenu;
    submenu->center(submenuPosition);
    m_submenus["main"] = submenu;


    // Options Submenu
    submenu = new Submenu(m_font);
    
    Vector titlePos = Vector(static_cast<float>(resX) / 2,
                             resY - static_cast<float>(resY) / 6, 
                             0);
    submenu->setTitle(language->get(TEXT_OPTIONS), titlePos);

    submenu->addEntry(new SubmenuEntry(language->get(TEXT_VIDEO_OPTIONS), this, "videoOptions"));
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_AUDIO_OPTIONS), this, "audioOptions"));
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_OTHER_OPTIONS), this, "otherOptions"));

    submenu->addEntry(new SpacerEntry());
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_BACK), this, "main"));    

    submenu->center(submenuPosition);
    m_submenus["options"] = submenu;


    // VIDEO Options Submenu
    submenu = new Submenu(m_font);
    
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
    submenu->addEntry(new OptionEntry(language->get(TEXT_SHADOWTYPE), valShad));

    Value valShadS("shadowmap_size");
    valShadS.add(L"512");
    valShadS.add(L"1024");
    valShadS.add(L"2048");
    submenu->addEntry(new OptionEntry(language->get(TEXT_SHADOWMAPSIZE), valShadS));

    BoolValue valFPS("show_fps");
    submenu->addEntry(new OptionEntry(language->get(TEXT_SHOWFPS), valFPS));

    submenu->addEntry(new SpacerEntry());
    submenu->addEntry(new ApplyOptionsEntry(language->get(TEXT_SAVE), this, "videoOptions"));
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_BACK), this, "options"));    
    
    submenu->center(submenuPosition);
    m_submenus["videoOptions"] = submenu;


    // AUDIO Options Submenu
    submenu = new Submenu(m_font);
    
    submenu->setTitle(language->get(TEXT_AUDIO_OPTIONS), titlePos);

    BoolValue valAud("audio");
    submenu->addEntry(new OptionEntry(language->get(TEXT_AUDIO), valAud));

    submenu->addEntry(new SpacerEntry());
    submenu->addEntry(new ApplyOptionsEntry(language->get(TEXT_SAVE), this, "audioOptions"));
    submenu->addEntry(new SubmenuEntry(language->get(TEXT_BACK), this, "options"));    
    
    submenu->center(submenuPosition);
    m_submenus["audioOptions"] = submenu;

    
    
    // OTHER Options Submenu
    submenu = new Submenu(m_font);
    
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
    m_submenus["otherOptions"] = submenu;


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
                // assume back is last entry
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
