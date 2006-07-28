#include "audio.h"
#include "game.h"
#include "config.h"
#include "common.h"
#include "music.h"

#include <AL/al.h>
#include <AL/alc.h>

Audio::Audio(const Game* game) : m_config(game->m_config.get())
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

    if (m_config->m_audio.enabled == false)
    {
        alcSuspendContext(m_context);
    }
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
    return *m_music.insert(new Music(filename)).first;
}

void Audio::unloadMusic(Music* music)
{
    m_music.erase(music);
    delete music;

}

void Audio::update()
{
    for each_(MusicSet, m_music, iter)
    {
        (*iter)->update();
    }
}
