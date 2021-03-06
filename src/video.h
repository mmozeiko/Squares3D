#ifndef __VIDEO_H__
#define __VIDEO_H__

#define Font FontX11
#include <GLee.h>
#undef min // WINAPI hacks
#undef max
#include <GL/glfw.h>
#undef Font

#include "common.h"
#include "vmath.h"
#include "system.h"

class Material;
class Texture;
class Collision;

struct UV
{
    UV(float u = 0.0f, float v = 0.0f) : u(u), v(v) {}

    float u, v;
};

struct Face
{
    vector<UV>     uv;
    vector<Vector> normal;
    vector<Vector> vertexes;
};

typedef map<string, Texture*> TextureMap;

void video_setup();
void video_finish();

class Video : public System<Video>, public NoCopy
{
public:
    Video();
    ~Video();

    void init();
    void unloadTextures();

    void renderFace(const Face& face) const;
    void renderAxes(float size = 5.0f) const;
    void renderRoundRect(const Vector& lower, const Vector& upper, float r) const;
    void renderSimpleShadow(float r, const Vector& pos, const Collision* level, const Vector& color) const;

    void begin() const;
    void begin(const Matrix& matrix) const;
    void end() const;

    void bind(const Material* material) const;

    IntPair getResolution() const;
    const IntPairVector& getModes() const;

    unsigned int newList();
    Texture* loadTexture(const string& name, bool mipmap = true);

    bool m_haveAnisotropy;
    int  m_maxAnisotropy;
    bool m_haveFBO;
    bool m_haveShadows;
    bool m_haveShaders;

    bool m_haveVBO;

private:
    void loadExtensions();

    TextureMap    m_textures;
    IntPair       m_resolution;    
    UIntSet       m_lists;

    vector<float> m_circleSin;
    vector<float> m_circleCos;
};

#endif
