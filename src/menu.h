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

class Submenu;
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
    State::Type progress();

    Music*      m_music;
    const Font* m_font;
    Submenu*    m_currentSubmenu;

    Submenus     m_submenus;
private:
    State::Type  m_state;
    Face*        m_backGround;
    Texture*     m_backGroundTexture;

};

#endif
