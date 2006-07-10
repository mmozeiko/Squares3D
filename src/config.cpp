#include "config.h"
#include "file.h"
#include "xml.h"

const string Config::CONFIG_FILE = "/config.xml";

const VideoConfig Config::defaultVideo = { 800, 600, false, false };
const AudioConfig Config::defaultAudio = { true };
const MiscConfig Config::defaultMisc = { true };

Config::Config() :
    m_video(defaultVideo), m_audio(defaultAudio), m_misc(defaultMisc)
{
    clog << "Reading configuration." << endl;

    XMLnode xml;
    File::Reader in(CONFIG_FILE);
    if (in.is_open())
    {
        in >> xml;
        in.close();
    }

    // TODO: maybe rewrite with map<string, variable&>
    // TODO: add line numbers in exceptions

    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "video")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "width")
                {
                    m_video.width = cast<string, int>(node.value);
                }
                else if (node.name == "height")
                {
                    m_video.height = cast<string, int>(node.value);
                }
                else if (node.name == "fullscreen")
                {
                    m_video.fullscreen = cast<string, int>(node.value)==1;
                }
                else if (node.name == "vsync")
                {
                    m_video.vsync = cast<string, int>(node.value)==1;
                }
                else
                {
                    throw Exception("Invalid configuration file, unknown video parameter - " + node.name);
                }
            }
        }
        else if (node.name == "audio")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "enabled")
                {
                    m_audio.enabled = cast<string, int>(node.value)==1;
                }
                else
                {
                    throw Exception("Invalid configuration file, unknown audio parameter - " + node.name);
                }
            }
        }
        else if (node.name == "misc")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "system_keys")
                {
                    m_misc.system_keys = cast<string, int>(node.value)==1;
                }
                else
                {
                    throw Exception("Invalid configuration file, unknown misc parameter - " + node.name);
                }
            }
        }
        else
        {
            throw Exception("Invalid configuration file, unknown section - " + node.name);
        }
    }
}

Config::~Config()
{
    clog << "Saving configuration." << endl;

    File::Writer out(CONFIG_FILE);
    if (!out.is_open())
    {
        throw Exception("Failed to open " + CONFIG_FILE + " for writing");
    }
    else
    {
        XMLnode video("video");
        video.childs.push_back(XMLnode("width", cast<int, string>(m_video.width)));
        video.childs.push_back(XMLnode("height", cast<int, string>(m_video.height)));
        video.childs.push_back(XMLnode("fullscreen", cast<int, string>(m_video.fullscreen ? 1 : 0)));
        video.childs.push_back(XMLnode("vsync", cast<int, string>(m_video.vsync ? 1 : 0)));

        XMLnode audio("audio");
        audio.childs.push_back(XMLnode("enabled", cast<int, string>(m_audio.enabled ? 1 : 0)));

        XMLnode misc("misc");
        misc.childs.push_back(XMLnode("system_keys", cast<int, string>(m_misc.system_keys ? 1 : 0)));

        XMLnode xml("config");
        xml.childs.push_back(video);
        xml.childs.push_back(audio);
        xml.childs.push_back(misc);

        out << xml;
        out.close();
    }
}

VideoConfig& Config::video()
{
    return m_video;
}

AudioConfig& Config::audio()
{
    return m_audio;
}

MiscConfig& Config::misc()
{
    return m_misc;
}
