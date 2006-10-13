#include <AL/al.h>

#include "sound_buffer.h"

SoundBuffer::SoundBuffer(const string& filename) : OggDecoder("/data/sound/" + filename + ".ogg")
{
    size_t bufferSize = totalSize();

    char* buffer = new char [bufferSize];

    alGenBuffers(1, &m_buffer);

    size_t written = decode(buffer, bufferSize);
    if (written != 0)
    {
        alBufferData(m_buffer, m_format, buffer, static_cast<int>(written), m_frequency);
    }
    delete [] buffer;
}

SoundBuffer::~SoundBuffer()
{
    alDeleteBuffers(1, &m_buffer);
}
