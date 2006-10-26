#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

#include "common.h"
#include "system.h"
#include "formatter.h"

// remember to update in Language constructor
enum TextType
{
    TEXT_START_SINGLEPLAYER,
    TEXT_EASY,
    TEXT_NORMAL,
    TEXT_HARD,
    TEXT_EXTRA,
    TEXT_MAIN_MENU,
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
    TEXT_ANISOTROPY,
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
    TEXT_CONTINUE,
    TEXT_RESTART,

    TEXT_FPS_DISPLAY,
    TEXT_GAME_OVER,
    TEXT_PLAYER_KICKS_OUT_BALL,
    TEXT_OUT_FROM_FIELD,
    TEXT_OUT_FROM_MIDDLE_LINE,
    TEXT_PLAYER_TOUCHES_TWICE,
    TEXT_PLAYER_UNALLOWED,
    TEXT_HITS_COMBO,
    TEXT_HITS,
    TEXT_SCORE_MESSAGE,
    TEXT_COLOR,
    TEXT_NAME,
    TEXT_PLAYER_OPTIONS,
    TEXT_RED,
    TEXT_GREEN,
    TEXT_BLUE,
    TEXT_BLACK,
    TEXT_WHITE,
    TEXT_YELLOW,
    TEXT_CYAN,
    TEXT_MAGENTA,
    TEXT_GREY,

    TEXT_SOUND_VOL,
    TEXT_MUSIC_VOL,

    TEXT_START_HOST,
    TEXT_START_JOIN,
    TEXT_START_MULTIPLAYER,
    TEXT_BEGIN_GAME,
    TEXT_PLAYERS_LIST,
    TEXT_CONNECT_HOST,
    TEXT_ADDRESS,
    TEXT_CONNECTING,
    TEXT_DISCONNECT,
    TEXT_LOCAL_PLAYER,
    TEXT_CPU_PLAYER,
    TEXT_REMOTE_PLAYER,
    TEXT_WAIT_PLAYERS,
	TEXT_CREDITS_SCREEN,

    TEXT_LAST_ONE, // THIS MUST BE TEH LAST ONE
};

typedef map<TextType, wstring> TextTypeMap;
typedef map<string, TextType> StrToTextTypeMap;

class Language : public System<Language>, public NoCopy
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
