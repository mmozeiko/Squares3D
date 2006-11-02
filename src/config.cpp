#include "config.h"
#include "file.h"
#include "xml.h"
#include "version.h"

template <class Config> Config* System<Config>::instance = NULL;

const string Config::CONFIG_FILE = "/config.xml";

const VideoConfig Config::defaultVideo = { 800, 600, false, true, 0, 0, 1, 1, false, 1, 1, false };
const AudioConfig Config::defaultAudio = { true, 5, 5 };
const MiscConfig Config::defaultMisc = { true, "en", 5.0f, "localhost" };

Config::Config() : m_video(defaultVideo), m_audio(defaultAudio), m_misc(defaultMisc)
{
    clog << "Reading configuration." << endl;

    XMLnode xml;
    File::Reader in(CONFIG_FILE);
    if (in.is_open())
    {
        xml.load(in);
        in.close();
    }

    string version = xml.getAttribute("version", string());
    
    if (version != g_version)
    {
        // config file version mismatch
        return;
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
                else if (node.name == "fsaa_samples")
                {
                    m_video.samples = cast<int>(node.value);
                    if (m_video.samples < 0 || m_video.samples > 8 || (m_video.samples&1) == 1)
                    {
                        m_video.samples = Config::defaultVideo.samples;
                    }
                }
                else if (node.name == "anisotropy")
                {
                    int i = cast<int>(node.value);
                    m_video.anisotropy = 0;
                    while (i > 1)
                    {
                        i >>= 1;
                        m_video.anisotropy++;
                    }
                }
                else if (node.name == "shadow_type")
                {
                    int shadow_type = cast<int>(node.value);
                    if (shadow_type < 0 || shadow_type > 1)
                    {
                        shadow_type = Config::defaultVideo.shadow_type;
                    }
                    m_video.shadow_type = shadow_type;
                }
                else if (node.name == "shadowmap_size")
                {
                    int shadowmap_size = cast<int>(node.value);
                    if (shadowmap_size != 0 &&
                        shadowmap_size != 1 &&
                        shadowmap_size != 2)
                    {
                        shadowmap_size = Config::defaultVideo.shadowmap_size;
                    }
                    m_video.shadowmap_size = shadowmap_size;
                }
                else if (node.name == "show_fps")
                {
                    m_video.show_fps = cast<int>(node.value)==1;
                }
                else if (node.name == "grass_density")
                {
                    int grass_density = cast<int>(node.value);
                    if (grass_density != 0 &&
                        grass_density != 1 &&
                        grass_density != 2)
                    {
                        grass_density = Config::defaultVideo.grass_density;
                    }
                    m_video.grass_density = grass_density;
                }
                else if (node.name == "terrain_detail")
                {
                    int terrain_detail = cast<int>(node.value);
                    if (terrain_detail != 0 &&
                        terrain_detail != 1 &&
                        terrain_detail != 2)
                    {
                        terrain_detail = Config::defaultVideo.terrain_detail;
                    }
                    m_video.terrain_detail = terrain_detail;
                }
                else if (node.name == "use_hdr")
                {
                    m_video.use_hdr = cast<int>(node.value)==1;
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
                else if (node.name == "music_vol")
                {
                    int music_vol = cast<int>(node.value);
                    if (music_vol < 0 || music_vol > 9)
                    {
                        music_vol = 5;
                    }
                    m_audio.music_vol = music_vol;
                }
                else if (node.name == "sound_vol")
                {
                    int sound_vol = cast<int>(node.value);
                    if (sound_vol < 0 || sound_vol > 9)
                    {
                        sound_vol = 5;
                    }
                    m_audio.sound_vol = sound_vol;
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
                else if (node.name == "language")
                {
                    m_misc.language = node.value;
                }
                else if (node.name == "mouse_sensitivity")
                {
                    float mouse_sensitivity = cast<float>(node.value);
                    if (mouse_sensitivity < 1.0f || mouse_sensitivity > 10.0f)
                    {
                        mouse_sensitivity = Config::defaultMisc.mouse_sensitivity;
                    }
                    mouse_sensitivity = static_cast<float>(static_cast<int>(2.0f * mouse_sensitivity)) / 2.0f;
                    m_misc.mouse_sensitivity = mouse_sensitivity;
                }
                else if (node.name == "last_address")
                {
                    m_misc.last_address = node.value;
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

    XMLnode xml("config");
    xml.setAttribute("version", g_version);

    xml.childs.push_back(XMLnode("video"));
    xml.childs.back().childs.push_back(XMLnode("width", cast<string>(m_video.width)));
    xml.childs.back().childs.push_back(XMLnode("height", cast<string>(m_video.height)));
    xml.childs.back().childs.push_back(XMLnode("fullscreen", cast<string>(m_video.fullscreen ? 1 : 0)));
    xml.childs.back().childs.push_back(XMLnode("vsync", cast<string>(m_video.vsync ? 1 : 0)));
    xml.childs.back().childs.push_back(XMLnode("fsaa_samples", cast<string>(m_video.samples)));
    xml.childs.back().childs.push_back(XMLnode("anisotropy", cast<string>(m_video.anisotropy == 0 ? 0 : 1 << m_video.anisotropy)));
    xml.childs.back().childs.push_back(XMLnode("shadow_type", cast<string>(m_video.shadow_type)));
    xml.childs.back().childs.push_back(XMLnode("shadowmap_size", cast<string>(m_video.shadowmap_size)));
    xml.childs.back().childs.push_back(XMLnode("show_fps", cast<string>(m_video.show_fps ? 1 : 0)));
    xml.childs.back().childs.push_back(XMLnode("grass_density", cast<string>(m_video.grass_density)));
    xml.childs.back().childs.push_back(XMLnode("terrain_detail", cast<string>(m_video.terrain_detail)));
    xml.childs.back().childs.push_back(XMLnode("use_hdr", cast<string>(m_video.use_hdr ? 1 : 0)));

    xml.childs.push_back(XMLnode("audio"));
    xml.childs.back().childs.push_back(XMLnode("enabled", cast<string>(m_audio.enabled ? 1 : 0)));
    xml.childs.back().childs.push_back(XMLnode("music_vol", cast<string>(m_audio.music_vol)));
    xml.childs.back().childs.push_back(XMLnode("sound_vol", cast<string>(m_audio.sound_vol)));

    xml.childs.push_back(XMLnode("misc"));
    xml.childs.back().childs.push_back(XMLnode("system_keys", cast<string>(m_misc.system_keys ? 1 : 0)));
    xml.childs.back().childs.push_back(XMLnode("language", m_misc.language));
    xml.childs.back().childs.push_back(XMLnode("last_address", m_misc.last_address));
    //xml.childs.back().childs.push_back(XMLnode("mouse_sensitivity", cast<string>(m_misc.mouse_sensitivity)));

    File::Writer out(CONFIG_FILE);
    if (!out.is_open())
    {
        throw Exception("Failed to open " + CONFIG_FILE + " for writing");
    }
    xml.save(out);
    out.close();
}
