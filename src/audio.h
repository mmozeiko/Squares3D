#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "common.h"

// forward declarations
typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;

class Game;
class Config;

class Music;
typedef set<Music*> MusicSet;

class Audio
{
public:
    Audio(const Game* game);
    ~Audio();

    Music* loadMusic(const string& filename);
    void unloadMusic(Music* music);

    void update();

private:
    const Config* m_config;
    ALCdevice*    m_device;
    ALCcontext*   m_context;

    MusicSet      m_music;
};

#endif
