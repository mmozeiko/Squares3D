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
    int  anisotropy;
    int  shadow_type;
    int  shadowmap_size;
    bool show_fps;
    int  grass_density;
    int  terrain_detail;
};

struct AudioConfig
{
    bool enabled;
    int  music_vol;
    int  sound_vol;
};

struct MiscConfig
{
    bool system_keys;
    string language;
    float mouse_sensitivity;
    string last_address;
};


class Config : public System<Config>, public NoCopy
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
