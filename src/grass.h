#ifndef __GRASS_H__
#define __GRASS_H__

#include "common.h"
#include "vmath.h"
#include "video.h"

class Level;
class Texture;

struct GrassFace
{
    GrassFace(const UV& uv, const Vector& vertex) : uv(uv), vertex(vertex) {}

    UV uv;
    Vector vertex;
};

class Grass
{
public:
    Grass(const Level* level);
    ~Grass();

    void update(float delta);
    void render() const;

private:
    float             m_time;

    size_t            m_count;
    unsigned int      m_buffer;
    vector<GrassFace> m_faces;
    Texture*          m_grassTex;  
};

#endif
