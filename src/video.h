#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "config.h"
#include "vmath.h"

class Video
{
public:
    Video(Config& config);
    ~Video();

    void RenderCube(float size = 1.0f) const;
    void RenderSphere(float radius = 1.0f) const;
    void RenderWireSphere(float radius = 1.0f) const;
    void RenderAxes(float size = 5.0f) const;

    void BeginObject(const Matrix& matrix) const;
    void EndObject() const;

private:
    Config& _config;
};

#endif
