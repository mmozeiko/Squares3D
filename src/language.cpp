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
    REGISTER_TEXT_TYPE(RED);
    REGISTER_TEXT_TYPE(GREEN);
    REGISTER_TEXT_TYPE(BLUE);
    REGISTER_TEXT_TYPE(BLACK);
    REGISTER_TEXT_TYPE(WHITE);
    REGISTER_TEXT_TYPE(YELLOW);
    REGISTER_TEXT_TYPE(CYAN);
    REGISTER_TEXT_TYPE(MAGENTA);
    REGISTER_TEXT_TYPE(GREY);

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

static wstring UTF8_to_UCS2(const string& str);

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
        m_lang[type] = UTF8_to_UCS2(value);
		wchar_t max = 0;
		for (size_t t=0; t<m_lang[type].size(); t++)
		{
		  if (m_lang[type][t] > max)
		  {
		    max = m_lang[type][t];
		  }
		}
        count++;
    }
    
    return count;
}

/*
 * Copyright 2001-2004 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

static bool isLegalUTF8(const string& source, size_t start, size_t length)
{
#if 1 || defined(__APPLE__)
    return true;
#else
    unsigned char a;
    start += length;
    switch (length)
    {
        default: return false;
        /* Everything else falls through when "true"... */
        case 4:
            if ((a = source[--start]) < 0x80 || a > 0xBF) return false;
        case 3:
            if ((a = source[--start]) < 0x80 || a > 0xBF) return false;
        case 2:
            if ((a = source[--start]) > 0xBF) return false;

            switch ((unsigned char)source[start])
            {
                /* no fall-through in this inner switch */
                case 0xE0: if (a < 0xA0) return false; break;
                case 0xED: if (a > 0x9F) return false; break;
                case 0xF0: if (a < 0x90) return false; break;
                case 0xF4: if (a > 0x8F) return false; break;
                default:   if (a < 0x80) return false;
            }

        case 1:
            if (static_cast<unsigned char>(source[start]) >= 0x80 && 
                static_cast<unsigned char>(source[start]) < 0xC2) return false;
    }
    
    if (static_cast<unsigned int>(source[start]) > 0xF4) return false;
    return true;
#endif
}

static const char trailingBytesForUTF8[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5,
};

static const unsigned int offsetsFromUTF8[6] = {
    0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

wstring UTF8_to_UCS2(const string& str)
{
    wstring result;
    size_t i = 0;
    while (i < str.size())
    {
        unsigned int ch = 0;
        size_t c = static_cast<size_t>(str[i]) & 0xFF;
        size_t extraBytesToRead = trailingBytesForUTF8[c];
        if (i + extraBytesToRead >= str.size())
        {
            // no chars to finish UTF-8 character
            break;
        }
        
        /* Do this check whether lenient or strict */
        if (! isLegalUTF8(str, i, extraBytesToRead+1))
        {
            // illegal UTF-8 character
            clog << "Illegan UTF-8!" << endl;
            break;
        }
        /*
        * The cases all fall through. See "Note A" below.
        */
        switch (extraBytesToRead)
        {
            case 5: ch += static_cast<unsigned int>(str[i++]) & 0xFF; ch <<= 6; /* remember, illegal UTF-8 */
            case 4: ch += static_cast<unsigned int>(str[i++]) & 0xFF; ch <<= 6; /* remember, illegal UTF-8 */
            case 3: ch += static_cast<unsigned int>(str[i++]) & 0xFF; ch <<= 6;
            case 2: ch += static_cast<unsigned int>(str[i++]) & 0xFF; ch <<= 6;
            case 1: ch += static_cast<unsigned int>(str[i++]) & 0xFF; ch <<= 6;
            case 0: ch += static_cast<unsigned int>(str[i++]) & 0xFF;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];

        if (ch <= 0xFFFFUL) /* Target is a character <= 0xFFFF */
        {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= 0xD800UL && ch <= 0xDFFFUL)
            {
                // illegal UTF-16 char
                break;
            }
            else
            {
                result.push_back(ch); /* normal case */
            }
        }
        else if (ch > 0x10FFFFUL)
        {
            // illegal UCS2
            break;
        }
        else
        {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            // illegal UCS2
            break;
        }
    }
    return result;
}
