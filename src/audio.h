#ifndef __AUDIO_H__
#define __AUDIO_H__

// forward declarations
typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;

class Game;
class Config;

class Audio
{
public:
    Audio(const Game* game);
    ~Audio();

private:
    const Config* m_config;
    ALCdevice*    m_device;
    ALCcontext*   m_context;
};

#endif
