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
class Sound;
class SoundBuffer;
class Game;
class Chat;

extern string g_neededVersion;

typedef map<string, Submenu*> Submenus;

class Menu : public State
{
public:
    Menu(Profile* userProfile, int unlockable, int& current);
    ~Menu();

    void control();
    void update(float delta) {} 
    void updateStep(float delta) {} 
    void prepare() {}
    void render() const;
    void loadMenu(Profile* userProfile, int unlockable, int& current);
    void setState(State::Type state);
    void setSubmenu(const string& submenuToSwitchTo);
    State::Type progress();

    Music*      m_music;
    const Font* m_fontSmall;
    const Font* m_font;
    const Font* m_fontBig;
    Submenu*    m_currentSubmenu;
    Submenus    m_submenus;

    Sound*       m_sound;
    Sound*       m_sound2;
    SoundBuffer* m_soundOver;
    SoundBuffer* m_soundClick;
    SoundBuffer* m_soundBackClick;
    SoundBuffer* m_soundChange;
    Vector       m_mousePrevPos;
    Chat*        m_chat;

private:
    State::Type  m_state;
    Face*        m_backGround;
    Texture*     m_backGroundTexture;
};

#endif
