#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "common.h"
#include "video.h"

class Texture;

class SkyBox : public NoCopy
{
public:
    SkyBox(const string& name);

    void render() const;

private:
    Texture* m_texture[6];
    Face     m_faces[6];
};

#endif
