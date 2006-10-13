#ifndef __SOUND_BUFFER_H__
#define __SOUND_BUFFER_H__

#include "common.h"
#include "oggDecoder.h"

class SoundBuffer : public OggDecoder
{
    friend class Sound;
    friend class Audio;

private:
    SoundBuffer(const string& filename);
    ~SoundBuffer();

    unsigned int m_buffer;
};

#endif
