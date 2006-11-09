#ifndef __MENU_ENTRIES_H__
#define __MENU_ENTRIES_H__

#include "common.h"
#include "vmath.h"
#include "timer.h"
#include "font.h"

class Menu;
class Submenu;
class Entry;

typedef vector<Entry*> Entries;
typedef vector<string> Values;

class Value
{
    friend class OptionEntry;
public:
    Value(const string& id) : m_current(0), m_id(id) {}
    void add(const string& str)                      { m_values.push_back(str); }
    
    string getCurrent() const;
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

class Entry : public NoCopy
{
public: 
    string      m_string;
    Vector      m_lowerLeft;
    Vector      m_upperRight;

    Entry(Menu* menu, const string& stringIn, Font::AlignType align = Font::Align_Center, const Font* font = NULL);
    virtual ~Entry() {}

    virtual void click(int button) = 0;
    virtual void onChar(int ch) {}
    virtual void onBackspace() {}
    virtual string getString() const                     { return m_string;   }
    virtual string getValueID() const                    { return "";         }
    virtual string getValue() const                      { return "";         }
    virtual int getCurrentValueIdx() const               { return -1;         }
    virtual void reset()                                 {}
    virtual int  getHeight()                             { return 1;          }

    bool isEnabled() const                               { return m_enabled;  }
    void disable()                                       { m_enabled = false; }
    virtual void enable()                                { m_enabled = true;  }

    bool isMouseOver(const Vector& mousePos) const;
    virtual void calculateBounds(const Vector& position);
    void setXBound(float minX, float maxX);
    virtual void render() const;
    virtual int getMaxLeftWidth() const;
    virtual int getMaxRightWidth() const;

    const Font* m_font;

    Font::AlignType m_align;
protected:
    bool m_enabled;
    Menu* m_menu;
    bool m_forBounds;
};

class SubmenuEntry : public Entry
{
public: 
    SubmenuEntry(Menu* menu, const string& stringIn, bool back, const string&  submenuToSwitchTo, const Font* font = NULL) :
      Entry(menu, stringIn, Font::Align_Center, font), m_back(back), m_submenuToSwitchTo(submenuToSwitchTo) {}

    virtual void click(int button); 

protected:
    bool   m_back;
    string m_submenuToSwitchTo;
};

class ColorEntry : public Entry
{
public: 
    ColorEntry(Menu* menu, const string& label, Vector& binding, const ColorValue& value) :
      Entry(menu, label), m_binding(binding), m_value(value)
    {
        m_value.setCurrent(binding); 
    }

    void render() const;
    void click(int button);

    virtual int getMaxRightWidth() const;
    
private:
    Vector&    m_binding;
    ColorValue m_value;
};

class WritableEntry : public Entry
{
public: 
    WritableEntry(Menu* menu, const string& label, string& binding, const Submenu* ownerSubmenu, int maxBindingSize = -1) :
      Entry(menu, label), m_ownerSubmenu(ownerSubmenu), m_binding(binding), m_maxBindingSize(maxBindingSize) {}

    void render() const;
    void click(int button);
    void onChar(int ch);
    void onBackspace();
    
    int getMaxRightWidth() const;

private:
    const Submenu* m_ownerSubmenu;
    string&        m_binding;
    Timer          m_timer;
    Timer          m_backTimer;
    int            m_maxBindingSize;
};

class WorldEntry : public Entry
{
public: 
    WorldEntry(Menu* menu, const string& stringIn, int switchTo, int& current) :
        Entry(menu, stringIn),
        m_switchTo(switchTo),
        m_current(current)
   {}
    
    void click(int button);

private:
    int   m_switchTo;
    int&  m_current;
};

class QuitEntry : public Entry
{
public: 
    QuitEntry(Menu* menu, const string& stringIn) : Entry(menu, stringIn) {}
    
    void click(int button);
};

class SpacerEntry : public Entry
{
public: 
    SpacerEntry(Menu* menu) : Entry(menu, "") { disable(); }
    void click(int button)     {}
    string getString() const   { return ""; }
    bool isEnabled() const     { return false; }
    void enable()              { }
};

// TODO: make common super class for two following

// used for displaying on server side
class NetPlayerEntry : public Entry
{
public: 
    NetPlayerEntry(Menu* menu, int idx) : Entry(menu, "MMMMMMMMMMMMMMM"), m_idx(idx) {}

    string getString() const;
    void click(int button);
    void render() const;

private:
    int    m_idx;
};

// used for displaying on client side
class NetRemotePlayerEntry : public Entry
{
public: 
    NetRemotePlayerEntry(Menu* menu, int idx) : Entry(menu, "MMMMMMMMMMMMMMM"), m_idx(idx) { disable(); }

    string getString() const;
    void click(int button);
    void render() const;

private:
    int    m_idx;
};

class LabelEntry : public Entry
{
public: 
    LabelEntry(Menu* menu, const string& label, Font::AlignType align = Font::Align_Center, const Font* font = NULL) :
      Entry(menu, label, align, font) { m_enabled = false; }

    string getString() const;
    void click(int button) {}
};

class FormatterEntry : public Entry
{
public: 
    FormatterEntry(Menu* menu, const string& label, Font::AlignType align = Font::Align_Center, const Font* font = NULL) :
      Entry(menu, label, align, font) { m_enabled = false; }

    string getString() const;
    void click(int button) {}
};

class NewHostEntry : public SubmenuEntry
{
public: 
    NewHostEntry(Menu* menu, const string& stringIn, const string& submenuToSwitchTo) :
        SubmenuEntry(menu, stringIn, false, submenuToSwitchTo) {}

    void click(int button); 
};

class JoinHostEntry : public SubmenuEntry
{
public: 
    JoinHostEntry(Menu* menu, const string& stringIn, const string& submenuToSwitchTo, Submenu* owner) :
        SubmenuEntry(menu, stringIn, false, submenuToSwitchTo), m_owner(owner) {}

    void click(int button); 
private:
    Submenu* m_owner;
};

class CloseHostEntry : public SubmenuEntry
{
public: 
    CloseHostEntry(Menu* menu, const string& stringIn, const string& submenuToSwitchTo) :
        SubmenuEntry(menu, stringIn, true, submenuToSwitchTo) {}

    void click(int button); 
};

class ConnectEntry : public SubmenuEntry
{
public: 
    ConnectEntry(Menu* menu, const string& label, const string& submenuToSwitchTo, Submenu* owner);
    void click(int button);

private:
    Submenu* m_owner;
};

class MultiWorldEntry : public Entry
{
public: 
    MultiWorldEntry(Menu* menu, const string& stringIn) :
        Entry(menu, stringIn)
   {}
    
    void click(int button);
};

#endif

