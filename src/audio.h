#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "config.h"

// forward declarations
typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;

class Audio
{
public:
    Audio(Config& config);
    ~Audio();

private:
    Config&     _config;
    ALCdevice*  _device;
    ALCcontext* _context;
};

#endif
