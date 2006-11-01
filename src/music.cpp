#include "openal_includes.h"

#include "music.h"
#include "config.h"

Music::Music(const string& filename) : OggDecoder("/data/music/" + filename + ".ogg")
{
    // m_bufferSize is for 250ms

    if (m_channels == 1) // 16-bit sample, 1 channel = 2 bytes per sample
    {
        m_bufferSize = m_frequency >> 1;
        m_bufferSize -= (m_bufferSize % 2);
    }
    else // if (m_channels == 2) // 16-bit sample, 2 channels = 4 bytes per sample
    {
        m_bufferSize = m_frequency;
        m_bufferSize -= (m_bufferSize % 4);
    }

    m_buffer = new char [m_bufferSize];

    alGenBuffers(BUFFER_COUNT, m_buffers);
    alGenSources(1, &m_source);

    init();
}

void Music::init()
{
    for (int i=0; i<BUFFER_COUNT; i++)
    {
        size_t written = decode(m_buffer, m_bufferSize);
        if (written != 0)
        {
            alBufferData(m_buffers[i], m_format, m_buffer, static_cast<int>(written), m_frequency);
            alSourceQueueBuffers(m_source, 1, &m_buffers[i]);
        }
    }
    m_position = 0.0f;
    alSourcef(m_source, AL_GAIN, 0);
}

Music::~Music()
{
    stop();
    alDeleteBuffers(BUFFER_COUNT, m_buffers);
    alDeleteSources(1, &m_source);
    delete [] m_buffer;
}

void Music::play(bool looping)
{
    if (Config::instance->m_audio.enabled)
    {
        alSourcePlay(m_source);
    }
    m_looping = looping;
}

void Music::stop()
{
    alSourceStop(m_source);
}

void Music::update()
{
    int processed = 0;
    alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);

    while (processed > 0)
    {
        unsigned int buffer = 0;
        alSourceUnqueueBuffers(m_source, 1, &buffer);
        m_position += 0.25f;

        size_t written = decode(m_buffer, m_bufferSize);
        if (written == 0 && m_looping)
        {
            reset();
            written = decode(m_buffer, m_bufferSize);
        }
        if (written > 0)
        {
            alBufferData(buffer, m_format, m_buffer, static_cast<int>(written), m_frequency);
            alSourceQueueBuffers(m_source, 1, &buffer);
        }

        processed--;
    }
    
    float f;
    alGetSourcef(m_source, AL_SEC_OFFSET, &f);

    if (m_position + f < 5.0f)
    {
        alSourcef(m_source, AL_GAIN, Config::instance->m_audio.music_vol/15.0f * (m_position + f)/5.0f);
    }

    /*
    int state = 0;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING)
    {
        int queued = 0;
        alGetSourcei(m_source, AL_BUFFERS_QUEUED, &queued);
        if (queued == AL_TRUE)
        {
            alSourcePlay(m_source);
        }
        else
        {
            if (m_looping)
            {
                m_file.seek(0);
                init();
                alSourcePlay(m_source);
            }
        }
    }
    */
}
