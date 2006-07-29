#ifndef __OGG_DECODER_H__
#define __OGG_DECODER_H__

#include "common.h"
#include "file.h"
#include <vorbis/vorbisfile.h>

class OggDecoder
{
public:
    OggDecoder(const string& filename);
    ~OggDecoder();

protected:
    unsigned int m_frequency;
    unsigned int m_channels;
    unsigned int m_format;

    size_t totalSize();
    size_t decode(char* buffer, const size_t bufferSize);

private:
    File::Reader m_file;
    OggVorbis_File m_oggFile;

    void close();
};

#endif
