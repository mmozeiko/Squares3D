#ifndef __MENU_SUBMENU_H__
#define __MENU_SUBMENU_H__

#include "common.h"
#include "vmath.h"

class Entry;
class Font;
class Menu;

typedef vector<Entry*> Entries;

class Submenu : NoCopy
{
public:

    Entries m_entries;
    size_t  m_activeEntry;
    float   m_height;
    Vector  m_centerPos;

    Submenu(Menu* menu) :
        m_activeEntry(0), m_height(0), m_title(L""),
        m_menu(menu) {}
    ~Submenu();

    void addEntry(Entry* entry);
    void center(const Vector& centerPos);
    void render() const;
    void control(int key);
    void onChar(int ch);
    void setTitle(const wstring& title, const Vector& position);
    void activateNextEntry(bool moveDown);

    Vector  m_upper;
    Vector  m_lower;

    wstring m_title;
    Vector  m_titlePos;

private:
    Vector      m_previousMousePos;

    Menu*       m_menu;
};

#endif
