#ifndef __MENU__H__
#define __MENU__H__

#include "common.h"
#include "state.h"
#include "vmath.h"

class Music;
class Game;
class Font;
struct Face;
class Texture;
class Menu;
class Entry;
class Submenu;
class Profile;
struct Unlockables;

typedef map<string, Submenu*> Submenus;

typedef vector<Entry*> Entries;

class Submenu : NoCopy
{
public:

    Entries m_entries;
    size_t  m_activeEntry;
    float   m_height;
    Vector  m_centerPos;

    Submenu(const Font* font, const Font* fontBig) :
        m_activeEntry(0), m_height(0), m_title(L""),
        m_font(font), m_fontBig(fontBig) {}
    ~Submenu();

    void addEntry(Entry* entry);
    void center(const Vector& centerPos);
    void render() const;
    void control(int key);
    void onChar(int ch);
    void setTitle(const wstring& title, const Vector& position);
    void activateNextEntry(bool moveDown);

    Vector m_upper;
    Vector m_lower;

    wstring m_title;
    Vector m_titlePos;

    const Font* m_font;
private:
    const Font* m_fontBig;

    Vector m_previousMousePos;
};

class Menu : public State
{
public:
    Menu(Profile* userProfile, Unlockables* unlockables);
    ~Menu();

    void control();
    void update(float delta) {} 
    void updateStep(float delta) {} 
    void prepare() {}
    void render() const;
    void loadMenu(Profile* userProfile, Unlockables* unlockables);
    void setState(State::Type state);
    void setSubmenu(const string& submenuToSwitchTo);
    State::Type progress();

    Music*      m_music;
    const Font* m_font;
    const Font* m_fontBig;
    Submenu*    m_currentSubmenu;
    Submenus    m_submenus;

private:
    State::Type  m_state;
    Face*        m_backGround;
    Texture*     m_backGroundTexture;

};



#endif
