#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "common.h"
#include "system.h"

// forward declarations
typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;

class Config;

class Music;
class Sound;
class SoundBuffer;

typedef set<Music*> MusicSet;
typedef map<string, SoundBuffer*> SoundBufferMap;

class Audio : public System<Audio>, public NoCopy
{
public:
    Audio();
    ~Audio();

    Music* loadMusic(const string& filename);
    void unloadMusic(Music* music);

    SoundBuffer* loadSound(const string& filename);
    void unloadSound(SoundBuffer* soundBuf);

    Sound* newSound(bool interrupt = true);

    void update();

private:
    ALCdevice*    m_device;
    ALCcontext*   m_context;

    MusicSet       m_music;
    SoundBufferMap m_soundBuf;
};

#endif
