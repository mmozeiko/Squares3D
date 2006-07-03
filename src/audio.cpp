#include "audio.h"
#include "common.h"

#include <AL/al.h>
#include <AL/alc.h>

Audio::Audio(Config& config) : _config(config)
{
    clog << "Initializing audio." << endl;

    _device = alcOpenDevice(NULL);
    if (_device==NULL)
    {
        throw Exception(alcGetString(NULL, alcGetError(NULL)));
    }

    _context = alcCreateContext(_device, NULL);
    if (_context==NULL)
    {
        string error = string(alcGetString(_device, alcGetError(_device)));
        alcCloseDevice(_device);
        throw Exception(error);
    }
    alcMakeContextCurrent(_context);

    int major=0, minor=0;
    alcGetIntegerv(_device, ALC_MAJOR_VERSION, 1, &major);
    alcGetIntegerv(_device, ALC_MINOR_VERSION, 1, &minor);

    clog << " * Version  : " << major << '.' << minor << endl
         << " * Vendor   : " << alGetString(AL_VENDOR) << endl
         << " * Renderer : " << alGetString(AL_RENDERER) << endl
         << " * Device   : " << alcGetString(_device, ALC_DEVICE_SPECIFIER) << endl;

    if (config["audio_on"]=="0")
    {
        alcSuspendContext(_context);
    }
}

Audio::~Audio()
{
    clog << "Closing audio." << endl;
    alcMakeContextCurrent(NULL);
    alcDestroyContext(_context);
    alcCloseDevice(_device);
}
