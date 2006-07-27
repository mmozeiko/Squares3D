#ifndef __SOUND_H__
#define __SOUND_H__

#include <vorbis/vorbisfile.h>
#include "file.h"
#include "vmath.h"

class Audio;

class Sound
{
    friend class Audio;

public:
    ~Sound();
    
    void play();
    void stop();
    bool is_playing();

    void update(const Vector& position, const Vector& velocity);

private:
    Sound(const string& filename, Audio* audio);

    void update();

    Audio* m_audio;
    unsigned int m_source;
    unsigned int m_buffer;

    File::Reader m_file;
    OggVorbis_File m_oggFile;

    int decode(char* buffer, size_t bufferSize);
};

#endif
