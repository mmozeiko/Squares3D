#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

#include "common.h"
#include "system.h"
#include "formatter.h"

// remember to update in Language constructor
enum TextType
{
    TEXT_START_GAME,
    TEXT_OPTIONS,
    TEXT_AUDIO_OPTIONS,
    TEXT_VIDEO_OPTIONS,
    TEXT_VIDEO_OPTIONS_ADVANCED,
    TEXT_OTHER_OPTIONS,
    TEXT_CREDITS,
    TEXT_QUIT_GAME,
    TEXT_BACK,
    TEXT_TRUE,
    TEXT_FALSE,
    TEXT_SAVE,
    TEXT_RESOLUTION,
    TEXT_FULLSCREEN,
    TEXT_VSYNC,
    TEXT_FSAA,
    TEXT_SHADERS,
    TEXT_SHADOW_TYPE,
    TEXT_SHADOWMAP_SIZE,
    TEXT_SHOW_FPS,
    TEXT_GRASS_DENSITY,
    TEXT_TERRAIN_DETAIL,
    TEXT_LOW,
    TEXT_MEDIUM,
    TEXT_HIGH,
    TEXT_CUSTOM,
    TEXT_NOT_SUPPORTED,
    TEXT_AUDIO,
    TEXT_LANGUAGE,
    TEXT_ENVIRONMENT_DETAILS,
    TEXT_SYSTEM_KEYS,
    TEXT_MOUSE_SENSITIVITY,

    TEXT_ESC_MESSAGE,

    TEXT_FPS_DISPLAY,
    TEXT_GAME_OVER,
    TEXT_PLAYER_KICKS_OUT_BALL,
    TEXT_OUT_FROM_FIELD,
    TEXT_OUT_FROM_MIDDLE_LINE,
    TEXT_PLAYER_TOUCHES_TWICE,
    TEXT_HITS_COMBO,
    TEXT_HITS,
    TEXT_SCORE_MESSAGE,
    TEXT_COLOR,
    TEXT_NAME,

    TEXT_LAST_ONE, // THIS MUST BE TEH LAST ONE
};

typedef map<TextType, wstring> TextTypeMap;
typedef map<string, TextType> StrToTextTypeMap;

class Language : public System<Language>, NoCopy
{
public:
    Language();
    Formatter get(TextType id);
    StringVector getAvailable() const;

private:
    int load(const string& name);
    TextTypeMap      m_lang;
    StrToTextTypeMap m_texts;
    
};

#endif
