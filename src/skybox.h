#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "common.h"

class Video;

class SkyBox
{
public:
    SkyBox(Video* video, const string& name);
    ~SkyBox();

    void render(const Video* video) const;

private:
    unsigned int textures[6];
};

#endif
