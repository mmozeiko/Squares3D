#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "config.h"

class Video
{
public:
    Video(Config& config);
    ~Video();

    void renderCube(float size = 1.0f) const;
    void renderSphere(float radius = 1.0f) const;
    void renderWireSphere(float radius = 1.0f) const;
    void renderAxes(float size = 5.0f) const;

private:
    Config& _config;
};

#endif
