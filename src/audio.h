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
typedef set<Sound*> SoundSet;
typedef set<SoundBuffer*> SoundBufferSet;

class Audio : public System<Audio>, NoCopy
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

    void updateVolume(int soundVol, int musicVol);

private:
    ALCdevice*    m_device;
    ALCcontext*   m_context;

    MusicSet       m_music;
    SoundSet       m_sound;
    SoundBufferSet m_soundBuf;
};

#endif
