#ifndef __SOUND_H__
#define __SOUND_H__

#include "common.h"
#include "oggDecoder.h"
#include "vmath.h"

class SoundBuffer;

class Sound : public NoCopy
{
public:
    Sound(bool interrupt);
    ~Sound();
    
    void play(const SoundBuffer* buffer);
    void stop();
    bool is_playing();

    void update(const Vector& position, const Vector& velocity);

private:

    bool         m_interrupt;
    unsigned int m_source;
};

#endif
