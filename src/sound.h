#ifndef __SOUND_H__
#define __SOUND_H__

#include "oggDecoder.h"
#include "vmath.h"

class Sound : public OggDecoder
{
    friend class Audio;

public:
    Sound(const string& filename);
    ~Sound();
    
    void play();
    void stop();
    bool is_playing();

    void update(const Vector& position, const Vector& velocity);

private:
    unsigned int m_source;
    unsigned int m_buffer;
};

#endif
