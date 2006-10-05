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

class Grass : NoCopy
{
public:
    Grass(const Level* level);
    ~Grass();

    void update(float delta);
    void render() const;

private:
    float             m_time;

    size_t            m_count;
    size_t            m_count2;
    unsigned int      m_buffer[2];
    vector<GrassFace> m_faces;
    vector<GrassFace> m_faces2;
    Texture*          m_grassTex;  
};

#endif
