#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <GL/glfw.h>

#include "common.h"
#include "vmath.h"
#include "system.h"

class Material;
class Texture;
class Collision;

static const Vector g_fieldLower(-25.0f, 0.0f, -25.0f);
static const Vector g_fieldUpper(25.0f, 0.0f, 25.0f);

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

    void renderCube() const;
    void renderFace(const Face& face) const;
    void renderSphere() const;
    void renderSphere(float radius) const;
    void renderCylinder(float radius, float height) const;
    void renderCone(float radius, float height) const;
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
    int m_maxAnisotropy;
    bool m_haveShadows;
    bool m_haveShadowsFB;

    bool m_haveVBO;

    bool m_shadowMap3ndPass;

private:
    void loadExtensions();

    GLUquadric*   m_quadricTexSphere;
    unsigned int  m_cylinderList;
    unsigned int  m_coneList;

    TextureMap    m_textures;
    IntPair       m_resolution;    
    UIntSet       m_lists;

    unsigned int  m_cubeList;

    vector<float> m_circleSin;
    vector<float> m_circleCos;

    mutable const Material* m_lastBound;
};

#endif
