#include <AL/al.h>
#include "oggDecoder.h"

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
    return 0;
}

static long ov_tell_func(void *datasource)
{
    File::Reader* reader = static_cast<File::Reader*>(datasource);
    return static_cast<long>(reader->tell());
}

static const ov_callbacks oggCallbacks = { ov_read_func, ov_seek_func, ov_close_func, ov_tell_func };


OggDecoder::OggDecoder(const string& filename) : m_file(File::Reader(filename))
{
    if (!m_file.is_open())
    {
        throw Exception("File '" + filename + "' not found");
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
        close();
        throw Exception("Error getting info from ogg file");
    }

    m_frequency = info->rate;
    m_channels = info->channels;

    if (m_channels == 1)
    {
        m_format = AL_FORMAT_MONO16;
    }
    else if (m_channels == 2)
    {
        m_format = AL_FORMAT_STEREO16;
    }
    else
    {
        close();
        throw Exception("Invalid channel count: " + cast<std::string>(m_channels));
    }
}

OggDecoder::~OggDecoder()
{
    close();
}

void OggDecoder::close()
{
    ov_clear(&m_oggFile);
    m_file.close();
}

size_t OggDecoder::totalSize()
{
    int total = static_cast<int>(ov_pcm_total(&m_oggFile, -1));
    if (total == OV_EINVAL)
    {
        close();
        throw Exception("Invalid bitstream");
    }
    return 2*m_channels*static_cast<size_t>(total);
}

size_t OggDecoder::decode(char* buffer, const size_t bufferSize)
{
    int current_section;
    size_t written = 0;

    while (written < bufferSize)
    {
        int decodeSize = ov_read(&m_oggFile, buffer + written, static_cast<int>(bufferSize - written), 0, 2, 1, &current_section);
        if (decodeSize <= 0)
        {
            break;
        }
        
        written += decodeSize;
    }
    return written;
}

void OggDecoder::reset()
{
    ov_raw_seek(&m_oggFile, 0);
}

