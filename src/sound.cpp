#include <AL/al.h>

#include "sound.h"

Sound::Sound(const string& filename) : OggDecoder("/data/sound/"+filename)
{
    size_t bufferSize = totalSize();

    char* buffer = new char [bufferSize];

    alGenBuffers(1, &m_buffer);
    alGenSources(1, &m_source);

    size_t written = decode(buffer, bufferSize);
    if (written != 0)
    {
        alBufferData(m_buffer, m_format, buffer, static_cast<int>(written), m_frequency);
        alSourcei(m_source, AL_BUFFER, m_buffer);
    }
    delete [] buffer;
}

Sound::~Sound()
{
    stop();
    alDeleteBuffers(1, &m_buffer);
    alDeleteSources(1, &m_source);
}

void Sound::play()
{
    alSourceRewind(m_source);
    alSourcePlay(m_source);
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
