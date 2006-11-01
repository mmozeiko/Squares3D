#ifndef __MENU_OPTIONS_H__
#define __MENU_OPTIONS_H__

#include "common.h"
#include "vmath.h"
#include "menu_entries.h"

class Font;
class Menu;

class OptionEntry : public Entry
{
public: 
    OptionEntry(Menu* menu, const wstring& stringIn, const Value& value) :
        Entry(menu, stringIn), m_value(value) {}

    void click(int button);
    wstring getString() const                      { return m_string +  L":  " + m_value.getCurrent(); }
    string getValueID() const                      { return m_value.m_id; }
    wstring getValue() const                       { return m_value.getCurrent(); }
    int getCurrentValueIdx() const                 { return static_cast<int>(m_value.m_current); }
    void reset();
    
    bool isMouseOver(const Vector& mousePos) const { return m_enabled && Entry::isMouseOver(mousePos); }
    
    void render() const;
    
    virtual int getMaxLeftWidth() const;
    virtual int getMaxRightWidth() const;

protected:
    Value m_value;
};

class ApplyOptionsEntry : public SubmenuEntry
{
public: 
    ApplyOptionsEntry(Menu* menu, const wstring& stringIn, const string&  submenuToSwitchTo) :
      SubmenuEntry(menu, stringIn, false, submenuToSwitchTo) {}

    void click(int button);
};

#endif
