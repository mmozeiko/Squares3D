#ifndef __AUDIO_H__
#define __AUDIO_H__

// forward declarations
typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;

class Game;

class Audio
{
public:
    Audio(const Game* game);
    ~Audio();

private:
    const Game* m_game;
    ALCdevice*  m_device;
    ALCcontext* m_context;
};

#endif
