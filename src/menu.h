#ifndef __MENU__H__
#define __MENU__H__

#include "common.h"
#include "state.h"
#include "system.h"
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

    wstring getCurrent();
    void addAnother(const wstring& string);
    Value(const string& id, const std::wstring& string);
    void activateNext();
};

class Entry
{
public: 
    wstring     m_string;
    Vector      m_position;
    const Font* m_font;
    Vector      m_lowerLeft;
    Vector      m_upperRight;

    Entry(const Vector& position, const wstring& stringIn, const Font* font);
    virtual void click() = 0;
    virtual wstring getString() = 0;
};

class OptionEntry : public Entry
{
public: 
    OptionEntry(const Vector& position, const wstring& stringIn, const Value& value, const Font* font);
    wstring getString();
    void click();
private:
    Value m_value;
};

class GameEntry : public Entry
{
public: 
    GameEntry(const Vector& position, 
              const wstring& stringIn, 
              Menu* menu, 
              State::Type stateToSwitchTo, 
              const Font* font);
    wstring getString();
    void click();
private:
    Menu* m_menu;
    State::Type m_stateToSwitchTo;

};

class SubmenuEntry : public Entry
{
public: 
    SubmenuEntry(const Vector&  position, 
                 const wstring& stringIn, 
                 Menu*          menu, 
                 const string&  submenuToSwitchTo, 
                 const   Font*  font);
    wstring getString();
    void click();
private:
    Menu*  m_menu;
    string m_submenuToSwitchTo;

};


typedef vector<Entry*> Entries;

class Submenu
{
public:

    Entries m_entries;
    Entry*  m_activeEntry;
    Vector  m_lastEntryPos;

    Submenu(Vector& lastEntryPos);
    ~Submenu();
    void addEntry(Entry* entry);
    void render() const;
    void control();
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
    void setSubmenu(string& submenuToSwitchTo);
    State::Type progress() const;

    Music*      m_music;
    Camera*     m_camera;
    const Font* m_font;

private:
    State::Type  m_state;
    Submenu*     m_currentSubmenu;
    Submenus     m_submenus;
    Face*        m_backGround;
    Texture*     m_backGroundTexture;

};

#endif
