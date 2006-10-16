#include <AL/al.h>
#include <AL/alc.h>

#include "audio.h"
#include "config.h"
#include "music.h"
#include "sound_buffer.h"
#include "sound.h"

template <class Audio> Audio* System<Audio>::instance = NULL;

Audio::Audio()
{
    clog << "Initializing audio." << endl;

    m_device = alcOpenDevice(NULL);
    if (m_device==NULL)
    {
        throw Exception(alcGetString(NULL, alcGetError(NULL)));
    }

    m_context = alcCreateContext(m_device, NULL);
    if (m_context==NULL)
    {
        string error = string(alcGetString(m_device, alcGetError(m_device)));
        alcCloseDevice(m_device);
        throw Exception(error);
    }
    alcMakeContextCurrent(m_context);

    int major=0, minor=0;
    alcGetIntegerv(m_device, ALC_MAJOR_VERSION, 1, &major);
    alcGetIntegerv(m_device, ALC_MINOR_VERSION, 1, &minor);

    clog << " * Version  : " << major << '.' << minor << endl
         << " * Vendor   : " << alGetString(AL_VENDOR) << endl
         << " * Renderer : " << alGetString(AL_RENDERER) << endl
         << " * Device   : " << alcGetString(m_device, ALC_DEVICE_SPECIFIER) << endl;
}

Audio::~Audio()
{
    clog << "Closing audio." << endl;
    alcMakeContextCurrent(NULL);
    alcDestroyContext(m_context);
    alcCloseDevice(m_device);
}

Music* Audio::loadMusic(const string& filename)
{
    Music* music = *m_music.insert(new Music(filename)).first;
    alSourcef(music->m_source, AL_GAIN, Config::instance->m_audio.music_vol/10.0f);
    return music;
}

void Audio::unloadMusic(Music* music)
{
    m_music.erase(music);
    delete music;
}

SoundBuffer* Audio::loadSound(const string& filename)
{
    return *m_soundBuf.insert(new SoundBuffer(filename)).first;
}

void Audio::unloadSound(SoundBuffer* soundBuf)
{
    m_soundBuf.erase(soundBuf);
    delete soundBuf;
}

Sound* Audio::newSound(bool interrupt)
{
    Sound* sound = *m_sound.insert(new Sound(interrupt)).first;
    alSourcef(sound->m_source, AL_GAIN, Config::instance->m_audio.sound_vol/10.0f);
    return sound;
}

void Audio::update()
{
    for each_(MusicSet, m_music, iter)
    {
        (*iter)->update();
    }
}

void Audio::updateVolume(int soundVol, int musicVol)
{
    for each_(SoundSet, m_sound, iter)
    {
        alSourcef((*iter)->m_source, AL_GAIN, soundVol/10.0f);
    }
}

