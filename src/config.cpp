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
                    m_video.width = cast<int>(node.value);
                }
                else if (node.name == "height")
                {
                    m_video.height = cast<int>(node.value);
                }
                else if (node.name == "fullscreen")
                {
                    m_video.fullscreen = cast<int>(node.value)==1;
                }
                else if (node.name == "vsync")
                {
                    m_video.vsync = cast<int>(node.value)==1;
                }
                else
                {
                    string line = cast<string>(node.line);
                    throw Exception("Invalid configuration file, unknown video parameter '" + node.name + "' at line " + line);
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
                    m_audio.enabled = cast<int>(node.value)==1;
                }
                else
                {
                    string line = cast<string>(node.line);
                    throw Exception("Invalid configuration file, unknown audio parameter '" + node.name + "' at line " + line);
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
                    m_misc.system_keys = cast<int>(node.value)==1;
                }
                else
                {
                    string line = cast<string>(node.line);
                    throw Exception("Invalid configuration file, unknown misc parameter '" + node.name + "' at line " + line);
                }
            }
        }
        else
        {
            string line = cast<string>(node.line);
            throw Exception("Invalid configuration file, unknown section '" + node.name + "' at line " + line);
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

    XMLnode xml("config");

    xml.childs.push_back(XMLnode("video"));
    xml.childs.back().childs.push_back(XMLnode("width", cast<string>(m_video.width)));
    xml.childs.back().childs.push_back(XMLnode("height", cast<string>(m_video.height)));
    xml.childs.back().childs.push_back(XMLnode("fullscreen", cast<string>(m_video.fullscreen ? 1 : 0)));
    xml.childs.back().childs.push_back(XMLnode("vsync", cast<string>(m_video.vsync ? 1 : 0)));

    xml.childs.push_back(XMLnode("audio"));
    xml.childs.back().childs.push_back(XMLnode("enabled", cast<string>(m_audio.enabled ? 1 : 0)));

    xml.childs.push_back(XMLnode("misc"));
    xml.childs.back().childs.push_back(XMLnode("system_keys", cast<string>(m_misc.system_keys ? 1 : 0)));

    out << xml;
    out.close();
}
