#include "music.h"
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

Music::Music(const string& filename, Audio* audio)
    : m_file(File::Reader("/data/music/"+filename)), m_audio(audio)

{
    if (!m_file.is_open())
    {
        throw Exception("Music file '" + filename + "' not found");
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

    m_frequency = info->rate;
    int channels = info->channels;

    if (channels == 1)
    {
        m_format = AL_FORMAT_MONO16;
        m_bufferSize = m_frequency >> 1;
        m_bufferSize -= (m_bufferSize % 2);
    }
    else if (channels == 2)
    {
        m_format = AL_FORMAT_STEREO16;
        m_bufferSize = m_frequency;
        m_bufferSize -= (m_bufferSize % 4);
    }
    else
    {
        ov_clear(&m_oggFile);
        throw Exception("Invalid channel count: " + cast<std::string>(channels));
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
        int written = decode();
        if (written != 0)
        {
            alBufferData(m_buffers[i], m_format, m_buffer, written, m_frequency);
            alSourceQueueBuffers(m_source, 1, &m_buffers[i]);
        }
    }
}

Music::~Music()
{
    stop();
    alDeleteBuffers(BUFFER_COUNT, m_buffers);
    alDeleteSources(1, &m_source);
    delete [] m_buffer;
    //m_audio->remove(this);
}

void Music::play(bool looping)
{
    alSourcePlay(m_source);
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

        int written = decode();
        if (written > 0)
        {
            alBufferData(buffer, m_format, m_buffer, written, m_frequency);
            alSourceQueueBuffers(m_source, 1, &buffer);
        }
        
        processed--;
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


int Music::decode()
{
    int current_section;
    int written = 0;

    while (true)
    {
        int decodeSize = ov_read(&m_oggFile, m_buffer + written, static_cast<int>(m_bufferSize) - written, 0, 2, 1, &current_section);
        if (decodeSize <= 0)
        {
            break;
        }
        
        written += decodeSize;

        if (written >= static_cast<int>(m_bufferSize))
        {
            break;
        }
    }
    return written;
}
