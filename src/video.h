#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "vmath.h"
#include "common.h"

#include <GL/glfw.h>

class Config;

class Video
{
public:
    Video(Config* config);
    ~Video();

    void renderCube(float size = 1.0f) const;
    void renderSphere(float radius = 1.0f) const;
    void renderWireSphere(float radius = 1.0f) const;
    void renderAxes(float size = 5.0f) const;

    void beginObject(const Matrix& matrix) const;
    void endObject() const;

    unsigned int loadTexture(const string& name) const;

private:
    Config* m_config;
};

#endif
