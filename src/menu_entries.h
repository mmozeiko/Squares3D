#ifndef __MENU_ENTRIES_H__
#define __MENU_ENTRIES_H__

#include "common.h"
#include "vmath.h"
#include "timer.h"

class Font;
class Menu;
class Submenu;
class Entry;

typedef vector<Entry*> Entries;
typedef vector<wstring> Values;

class Value
{
    friend class OptionEntry;
public:
    Value(const string& id) : m_current(0), m_id(id) {}
    void add(const wstring& string)                  { m_values.push_back(string); }
    
    wstring getCurrent() const;
    void activateNext(bool forward);
    int getMaxWidth(const Font* font) const;

protected:
    Values m_values;
    size_t m_current;
    string m_id;
};

class ColorValue : public Value
{
public:
    ColorValue(const string& id);
    void setCurrent(const Vector& color);
};

class Entry : NoCopy
{
public: 
    wstring     m_string;
    Vector      m_lowerLeft;
    Vector      m_upperRight;

    Entry(const wstring& stringIn) : m_string(stringIn), m_enabled(true) {}
    virtual ~Entry() {}

    virtual void click(int button) = 0;
    virtual void onChar(int ch) {}
    virtual wstring getString() const                    { return m_string; }
    virtual string getValueID() const                    { return "";       }
    virtual wstring getValue() const                     { return L"";      }
    virtual int getCurrentValueIdx() const               { return -1;       }
    virtual void reset()                                 {}
    bool isEnabled() const                               { return m_enabled;}
    void disable()                                       { m_enabled = false;}
    void enable()                                        { m_enabled = true;}

    bool isMouseOver(const Vector& mousePos) const;
    void calculateBounds(const Vector& position, const Font* font);
    void setXBound(float minX, float maxX);
    virtual void render(const Font* font) const;
    virtual int getMaxLeftWidth(const Font* font) const;
    virtual int getMaxRightWidth(const Font* font) const;

protected:
    bool m_enabled;
};

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

class ColorEntry : public Entry
{
public: 
    ColorEntry(const wstring& label, Vector& binding, const ColorValue& value) :
      Entry(label), m_binding(binding), m_value(value)
    {
        m_value.setCurrent(binding); 
    }

    void render(const Font* font) const;
    void click(int button);

    virtual int getMaxRightWidth(const Font* font) const;
    
private:
    Vector&    m_binding;
    ColorValue m_value;
};

class WritableEntry : public Entry
{
public: 
    WritableEntry(const wstring& label, string& binding, const Submenu* ownerSubmenu) :
      Entry(label), m_ownerSubmenu(ownerSubmenu),  m_binding(binding) {}

    void render(const Font* font) const;
    void click(int button);
    void onChar(int ch);
    
    int getMaxRightWidth(const Font* font) const;

private:
    const Submenu* m_ownerSubmenu;
    string&        m_binding;
    Timer          m_timer;
};

class WorldEntry : public Entry
{
public: 
    WorldEntry(const wstring& stringIn, Menu* menu, int switchTo, int& current) :
        Entry(stringIn),
        m_menu(menu),
        m_switchTo(switchTo),
        m_current(current)
   {}
    
    void click(int button);

private:
    int   m_switchTo;
    int&  m_current;
    Menu* m_menu;
};

class QuitEntry : public Entry
{
public: 
    QuitEntry(const wstring& stringIn, Menu* menu) : Entry(stringIn), m_menu(menu) {}
    
    void click(int button);

private:
    Menu* m_menu;
};

class SpacerEntry : public Entry
{
public: 
    SpacerEntry() : Entry(L"") {}
    void click(int button)     {}
    wstring getString() const  { return L""; }
    bool isEnabled() const     { return false; }
};

#endif
