#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "common.h"

class Texture;

class SkyBox : NoCopy
{
public:
    SkyBox();

    void render() const;

private:
    Texture* m_texture[6];
};

#endif
