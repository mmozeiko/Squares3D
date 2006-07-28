#ifndef __MUSIC_H__
#define __MUSIC_H__

#include "oggDecoder.h"

#define BUFFER_COUNT 4

class Music : public OggDecoder
{
    friend class Audio;

public:
    void play(bool looping = true);
    void stop();

private:
    Music(const string& filename);
    ~Music();

    unsigned int m_source;
    unsigned int m_buffers[BUFFER_COUNT];
    char* m_buffer;
    size_t m_bufferSize;

    bool m_looping;

    void update();
    void init();
};

#endif
