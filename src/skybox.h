#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "common.h"

class Video;
class Texture;

class SkyBox : NoCopy
{
public:
    SkyBox(Video* video);

    void render(const Video* video) const;

private:
    Texture* m_texture;
};

#endif
