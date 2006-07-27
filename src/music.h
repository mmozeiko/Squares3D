#ifndef __MUSIC_H__
#define __MUSIC_H__

#include <vorbis/vorbisfile.h>
#include "file.h"

#define BUFFER_COUNT 4

class Audio;

class Music
{
    friend class Audio;

public:
    ~Music();
    void play(bool looping = true);
    void stop();

private:
    Music(const string& filename, Audio* audio);

    void update();

    Audio* m_audio;
    unsigned int m_source;
    unsigned int m_buffers[BUFFER_COUNT];
    char* m_buffer;
    size_t m_bufferSize;

    unsigned int m_frequency;
    unsigned int m_format;
    bool m_looping;

    File::Reader m_file;
    OggVorbis_File m_oggFile;

    void init();
    int decode();
};

#endif
