#include "sound.h"
#include "common.h"
#include <AL/al.h>
#include "audio.h"

static size_t ov_read_func(void* ptr, size_t size, size_t nmemb, void* datasource)
{
    File::Reader* reader = static_cast<File::Reader*>(datasource);
    if (reader->eof())
    {
        return 0;
    }
    return reader->read(ptr, size*nmemb)/size;
}

static int ov_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
    File::Reader* reader = static_cast<File::Reader*>(datasource);
    reader->seek(static_cast<int>(offset), whence);
    return 0;
}

static int ov_close_func(void *datasource)
{
    //File::Reader* reader = static_cast<File::Reader*>(datasource);
    //reader->close();
    return 0;
}

static long ov_tell_func(void *datasource)
{
    File::Reader* reader = static_cast<File::Reader*>(datasource);
    return static_cast<long>(reader->tell());
}

static const ov_callbacks oggCallbacks = { ov_read_func, ov_seek_func, ov_close_func, ov_tell_func };

Sound::Sound(const string& filename, Audio* audio)
    : m_file(File::Reader("/data/sound/"+filename)), m_audio(audio)

{
    if (!m_file.is_open())
    {
        throw Exception("Sound file '" + filename + "' not found");
    }

    int result = ov_open_callbacks(static_cast<void*>(&m_file), &m_oggFile, NULL, 0, oggCallbacks);
    if (result != 0)
    {
        switch (result)
        {
          case OV_EREAD:      throw Exception("A read from media returned an error");
          case OV_ENOTVORBIS: throw Exception("Bitstream is not Vorbis data");
          case OV_EVERSION:   throw Exception("Vorbis version mismatch");
          case OV_EBADHEADER: throw Exception("Invalid Vorbis bitstream header");
          case OV_EFAULT:     throw Exception("Internal logic fault");
          default:            throw Exception("Unknown ogg open error");
        }
    }

    vorbis_info* info = ov_info(&m_oggFile, -1);
    if (info == NULL)
    {
        ov_clear(&m_oggFile);
        throw Exception("Error getting info from ogg file");
    }

    int format;
    int frequency = info->rate;
    int channels = info->channels;
    size_t bufferSize;

    if (channels == 1)
    {
        format = AL_FORMAT_MONO16;
        bufferSize = frequency >> 1;
        bufferSize -= (bufferSize % 2);
    }
    else if (channels == 2)
    {
        format = AL_FORMAT_STEREO16;
        bufferSize = frequency;
        bufferSize -= (bufferSize % 4);
    }
    else
    {
        ov_clear(&m_oggFile);
        throw Exception("Invalid channel count: " + cast<std::string>(channels));
    }

    char* buffer = new char [bufferSize];

    alGenBuffers(1, &m_buffer);
    alGenSources(1, &m_source);

    int written = decode(buffer, bufferSize);
    if (written != 0)
    {
        alBufferData(m_buffer, format, buffer, written, frequency);
        alSourcei(m_source, AL_BUFFER, m_buffer);
    }
    delete [] buffer;
}

Sound::~Sound()
{
    stop();
    alDeleteBuffers(1, &m_buffer);
    alDeleteSources(1, &m_source);
    //m_audio->remove(this);
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

int Sound::decode(char* buffer, size_t bufferSize)
{
    int current_section;
    int written = 0;

    while (true)
    {
        int decodeSize = ov_read(&m_oggFile, buffer + written, static_cast<int>(bufferSize) - written, 0, 2, 1, &current_section);
        if (decodeSize <= 0)
        {
            break;
        }
        
        written += decodeSize;

        if (written >= static_cast<int>(bufferSize))
        {
            break;
        }
    }
    return written;
}
