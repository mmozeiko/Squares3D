#include "language.h"
#include "xml.h"
#include "file.h"
#include "config.h"
#include <physfs.h>

template <class Language> Language* System<Language>::instance = NULL;

Language::Language()
{
    clog << "Initializing language texts... ";

#define REGISTER_TEXT_TYPE(type) m_texts.insert(make_pair(STR(type), TEXT_##type))

    REGISTER_TEXT_TYPE(FINISHED_GAME);
    REGISTER_TEXT_TYPE(START_SINGLEPLAYER);
    REGISTER_TEXT_TYPE(EASY);
    REGISTER_TEXT_TYPE(NORMAL);
    REGISTER_TEXT_TYPE(HARD);
    REGISTER_TEXT_TYPE(EXTRA);
    REGISTER_TEXT_TYPE(FPS_DISPLAY);
    REGISTER_TEXT_TYPE(GAME_OVER);
    REGISTER_TEXT_TYPE(PLAYER_KICKS_OUT_BALL);
    REGISTER_TEXT_TYPE(OUT_FROM_FIELD);
    REGISTER_TEXT_TYPE(OUT_FROM_MIDDLE_LINE);
    REGISTER_TEXT_TYPE(PLAYER_TOUCHES_TWICE);
    REGISTER_TEXT_TYPE(PLAYER_UNALLOWED);
    REGISTER_TEXT_TYPE(HITS_COMBO);
    REGISTER_TEXT_TYPE(HITS);
    REGISTER_TEXT_TYPE(SCORE_MESSAGE);
    REGISTER_TEXT_TYPE(ESC_MESSAGE);
    REGISTER_TEXT_TYPE(CONTINUE);
    REGISTER_TEXT_TYPE(RESTART);

    REGISTER_TEXT_TYPE(MAIN_MENU);
    REGISTER_TEXT_TYPE(OPTIONS);
    REGISTER_TEXT_TYPE(AUDIO_OPTIONS);
    REGISTER_TEXT_TYPE(VIDEO_OPTIONS);
    REGISTER_TEXT_TYPE(VIDEO_OPTIONS_ADVANCED);
    REGISTER_TEXT_TYPE(OTHER_OPTIONS);
    REGISTER_TEXT_TYPE(CREDITS);
    REGISTER_TEXT_TYPE(BACK);
    REGISTER_TEXT_TYPE(QUIT_GAME);
    REGISTER_TEXT_TYPE(TRUE);
    REGISTER_TEXT_TYPE(FALSE);
    REGISTER_TEXT_TYPE(SAVE);
    REGISTER_TEXT_TYPE(RESOLUTION);
    REGISTER_TEXT_TYPE(FULLSCREEN);
    REGISTER_TEXT_TYPE(VSYNC);
    REGISTER_TEXT_TYPE(FSAA);
    REGISTER_TEXT_TYPE(ANISOTROPY);
    REGISTER_TEXT_TYPE(SHADOW_TYPE);
    REGISTER_TEXT_TYPE(SHADOWMAP_SIZE);
	REGISTER_TEXT_TYPE(USE_HDR);
    REGISTER_TEXT_TYPE(SHOW_FPS);
    REGISTER_TEXT_TYPE(GRASS_DENSITY);
    REGISTER_TEXT_TYPE(TERRAIN_DETAIL);
    REGISTER_TEXT_TYPE(LOW);
    REGISTER_TEXT_TYPE(MEDIUM);
    REGISTER_TEXT_TYPE(HIGH);
    REGISTER_TEXT_TYPE(CUSTOM);
    REGISTER_TEXT_TYPE(NOT_SUPPORTED);
    REGISTER_TEXT_TYPE(AUDIO);
    REGISTER_TEXT_TYPE(LANGUAGE);
    REGISTER_TEXT_TYPE(ENVIRONMENT_DETAILS);
    REGISTER_TEXT_TYPE(MOUSE_SENSITIVITY);
    REGISTER_TEXT_TYPE(SYSTEM_KEYS);
    REGISTER_TEXT_TYPE(NAME);
    REGISTER_TEXT_TYPE(COLOR);
    REGISTER_TEXT_TYPE(PLAYER_OPTIONS);

    REGISTER_TEXT_TYPE(SOUND_VOL);
    REGISTER_TEXT_TYPE(MUSIC_VOL);

    REGISTER_TEXT_TYPE(START_HOST);
    REGISTER_TEXT_TYPE(START_JOIN);
    REGISTER_TEXT_TYPE(START_MULTIPLAYER);
    REGISTER_TEXT_TYPE(BEGIN_GAME);
    REGISTER_TEXT_TYPE(PLAYERS_LIST);
    REGISTER_TEXT_TYPE(CONNECT_HOST);
    REGISTER_TEXT_TYPE(ADDRESS);
    REGISTER_TEXT_TYPE(CONNECTING);
    REGISTER_TEXT_TYPE(DISCONNECT);
    REGISTER_TEXT_TYPE(LOCAL_PLAYER);
    REGISTER_TEXT_TYPE(CPU_PLAYER);
    REGISTER_TEXT_TYPE(REMOTE_PLAYER);
    REGISTER_TEXT_TYPE(WAIT_PLAYERS);

    REGISTER_TEXT_TYPE(LANG_ENGLISH);
    REGISTER_TEXT_TYPE(LANG_LATVIAN);
    REGISTER_TEXT_TYPE(LANG_RUSSIAN);
    REGISTER_TEXT_TYPE(LANG_GERMAN);

    REGISTER_TEXT_TYPE(CREDITS_SCREEN);
	REGISTER_TEXT_TYPE(RULES);
	REGISTER_TEXT_TYPE(RULES_SCREEN);
    REGISTER_TEXT_TYPE(RULES);
	REGISTER_TEXT_TYPE(RULES_SCREEN);
    REGISTER_TEXT_TYPE(CONTROLS);
	REGISTER_TEXT_TYPE(CONTROLS_SCREEN);	


#undef REGISTER_TEXT_TYPE
    
    assert(m_texts.size() == static_cast<int>(TEXT_LAST_ONE));

    // load default texts
    int count = load("en");
    if (count != static_cast<int>(m_texts.size()))
    {
        throw Exception("Default language file (en.xml) has invalid count of messages");  
    }
    
    // check if language is available
    StringVector available = getAvailable();
    if (!foundIn(available, Config::instance->m_misc.language))
    {
        Config::instance->m_misc.language = "en";
        return;
    }

    clog << "loading " << Config::instance->m_misc.language << " language." << endl;

    // load selected language
    int selCount = load(Config::instance->m_misc.language);
    if (selCount != count)
    {
        clog << "Warning: expected " << count << " phrases, but found only " << selCount << "!" << endl;
    }
}

StringVector Language::getAvailable() const
{
    StringVector result;

    char** files = PHYSFS_enumerateFiles("/data/language/");
    for (char** i = files; *i != NULL; i++)
    {
        string name = *i;
        if (name.size() > 4 && name.substr(name.size()-4, 4) == ".xml")
        {
            result.push_back(name.substr(0, name.size()-4));
        }
    }
    PHYSFS_freeList(files);

    return result;
}

Formatter Language::get(TextType id)
{
    return Formatter(m_lang.find(id)->second);
}

int Language::load(const string& name)
{
    string filename = "/data/language/" + name + ".xml";

    int count = 0;

    XMLnode xml;
    File::Reader in(filename);
    if (!in.is_open())
    {
        throw Exception("Language file '" + filename + "' not found");  
    }
    xml.load(in);
    in.close();

    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name != "item")
        {
            throw Exception("Invalid language file '" + filename + "'");
        }

        StrToTextTypeMap::const_iterator txtIter = m_texts.find(node.getAttribute("name"));
        if (txtIter == m_texts.end())
        {
            throw Exception("Invalid language file, unexpected '" + node.getAttribute("name") + "' item");
        }

        TextType type = txtIter->second;
        string value = (node.hasAttribute("value") ? node.getAttribute("value") : node.value);
        m_lang[type] = value;

        count++;
    }
    
    return count;
}
