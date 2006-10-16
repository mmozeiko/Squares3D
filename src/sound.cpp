#include <AL/al.h>

#include "sound.h"
#include "sound_buffer.h"
#include "config.h"

Sound::Sound(bool interrupt) :
    m_interrupt(interrupt)
{
    alGenSources(1, &m_source);
}

Sound::~Sound()
{
    stop();
    alDeleteSources(1, &m_source);
}

void Sound::play(const SoundBuffer* buffer)
{
    if (m_interrupt)
    {
        alSourceStop(m_source);
        alSourcei(m_source, AL_BUFFER, buffer->m_buffer);
        if (Config::instance->m_audio.enabled)
        {
            alSourcePlay(m_source);
        }
    }
    else if (!is_playing())
    {
        alSourcei(m_source, AL_BUFFER, buffer->m_buffer);
        if (Config::instance->m_audio.enabled)
        {
            alSourcePlay(m_source);
        }
    }
}

void Sound::stop()
{
    alSourceStop(m_source);
}

bool Sound::is_playing()
{
    int result;
    alGetSourcei(m_source, AL_SOURCE_STATE, &result);
    return result == AL_PLAYING;
}

void Sound::update(const Vector& position, const Vector& velocity)
{
    alSourcefv(m_source, AL_POSITION, position.v);
    alSourcefv(m_source, AL_VELOCITY, velocity.v);
}
