#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

#include "common.h"
#include "system.h"

// remember to update in Language constructor
enum TextType
{
    TEXT_FPS_DISPLAY,
    TEXT_GAME_OVER,
    TEXT_PLAYER_KICKS_OUT_BALL,
    TEXT_OUT_FROM_FIELD,
    TEXT_OUT_FROM_MIDDLE_LINE,
    TEXT_PLAYER_TOUCHES_TWICE,
    TEXT_HITS_COMBO,

    TEXT_LAST_ONE, // THIS MUST BE TEH LAST ONE
};

typedef map<TextType, wstring> TextTypeMap;
typedef map<string, TextType> StrToTextTypeMap;
class Formatter
{
    friend class Language;
public:
    Formatter& operator () (const string& value);
    Formatter& operator () (float value);
    Formatter& operator () (int value);
    operator wstring ();

private:
    Formatter(const wstring& txt);
    wstring m_txt;
};

class Language : public System<Language>
{
public:
    Language();
    Formatter get(TextType id);
    StringVector getAvailable() const;

private:
    void load(const string& name);
    TextTypeMap      m_lang;
    StrToTextTypeMap m_texts;
    
};

#endif
