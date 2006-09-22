#ifndef __MENU__H__
#define __MENU__H__

#include "common.h"
#include "state.h"
#include "vmath.h"

class Camera;
class Music;
class Game;
class Font;
struct Face;
class Texture;
class Menu;

typedef vector<wstring> Values;
class Value
{
public:
    Values m_values;
    size_t m_current;
    string m_id;

    Value(const string& id);
    wstring getCurrent() const;
    void addAnother(const wstring& string);
    void activateNext();
};

class BoolValue : public Value
{
public:
    BoolValue(const string& id); 
    void addAnother(const wstring& string);
};

class Entry
{
public: 
    wstring     m_string;
    const Font* m_font;
    Vector      m_lowerLeft;
    Vector      m_upperRight;

    Entry(const wstring& stringIn, const Font* font);
    void calculateBounds(const Vector& position);
    virtual ~Entry() {}
    virtual void click() = 0;
    virtual wstring getString() const = 0;
    virtual string getValueID() const;
    virtual wstring getValue() const;
    virtual size_t getCurrentValueIdx() const;
    virtual bool isEnabled() const;
    bool isMouseOver(const Vector& mousePos) const;
    virtual void reset() {}
};

class OptionEntry : public Entry
{
public: 
    OptionEntry(const wstring& stringIn, const Value& value, const Font* font);
    wstring getString() const;
    string getValueID() const;
    size_t getCurrentValueIdx() const;
    wstring getValue() const;
    bool isEnabled() const;
    bool isMouseOver(const Vector& mousePos) const;
    void click();
    void reset();
private:
    Value m_value;
    bool m_enabled;
};

class GameEntry : public Entry
{
public: 
    GameEntry(const wstring& stringIn, 
              Menu* menu, 
              State::Type stateToSwitchTo, 
              const Font* font);
    wstring getString() const;
    void click();
private:
    Menu* m_menu;
    State::Type m_stateToSwitchTo;

};

class SubmenuEntry : public Entry
{
public: 
    SubmenuEntry(const wstring& stringIn, 
                 Menu*          menu, 
                 const string&  submenuToSwitchTo, 
                 const   Font*  font);
    wstring getString() const;
    void click();
protected:
    Menu*  m_menu;
    string m_submenuToSwitchTo;
};

class ApplyOptionsEntry : public SubmenuEntry
{
public: 
    ApplyOptionsEntry(const wstring& stringIn, 
                      Menu*          menu, 
                      const string&  submenuToSwitchTo, 
                      const   Font*  font);
    void click();
};


typedef vector<Entry*> Entries;

class Submenu
{
public:

    Entries m_entries;
    size_t  m_activeEntry;
    float   m_height;
    Vector  m_centerPos;

    Submenu();
    ~Submenu();
    void addEntry(Entry* entry);
    void center(const Vector& centerPos);
    void render() const;
    void control();
    void setTitle(const wstring& title, const Vector& position, const Font* font);
    void activateNextEntry(bool moveDown);

private:
    wstring m_title;
    Vector m_titlePos;
    const Font* m_titleFont;

    Vector m_previousMousePos;
};

typedef map<string, Submenu*> Submenus;

class Menu : public State
{
public:
    Menu();
    ~Menu();

    void control();
    void update(float delta);
    void updateStep(float delta);
    void prepare();
    void render() const;
    void loadMenu();
    void setState(State::Type state);
    void setSubmenu(const string& submenuToSwitchTo);
    State::Type progress() const;

    Music*      m_music;
    Camera*     m_camera;
    const Font* m_font;
    Submenu*    m_currentSubmenu;

    Submenus     m_submenus;
private:
    State::Type  m_state;
    Face*        m_backGround;
    Texture*     m_backGroundTexture;

};

#endif
