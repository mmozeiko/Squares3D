#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "common.h"
#include "system.h"

// forward declarations
typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;

class Config;

class Music;
typedef set<Music*> MusicSet;

class Audio : public System<Audio>, NoCopy
{
public:
    Audio();
    ~Audio();

    Music* loadMusic(const string& filename);
    void unloadMusic(Music* music);

    void update();

private:
    ALCdevice*    m_device;
    ALCcontext*   m_context;

    MusicSet      m_music;
};

#endif
