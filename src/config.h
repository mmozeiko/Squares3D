#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "common.h"
#include "system.h"

struct VideoConfig
{
    int  width;
    int  height;
    bool fullscreen;
    bool vsync;
    int  samples;
    bool use_shaders;
    int  shadow_type;
    int  shadowmap_size;
    bool show_fps;
    int  grass_density;
    int  terrain_detail;
};

struct AudioConfig
{
    bool enabled;
};

struct MiscConfig
{
    bool system_keys;
    string language;
};


class Config : public System<Config>, NoCopy
{
public:
    Config();
    ~Config();

    VideoConfig m_video;
    AudioConfig m_audio;
    MiscConfig  m_misc;

private:
    static const string CONFIG_FILE;
    static const VideoConfig defaultVideo;
    static const AudioConfig defaultAudio;
    static const MiscConfig defaultMisc;
};

#endif
