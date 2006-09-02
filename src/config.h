#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "common.h"

struct VideoConfig
{
    int width;
    int height;
    bool fullscreen;
    bool vsync;
    int samples;
};

struct AudioConfig
{
    bool enabled;
};

struct MiscConfig
{
    bool system_keys;
};


class Config : NoCopy
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
