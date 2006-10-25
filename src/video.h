#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <GL/glfw.h>
#include <GL/glext.h>

#include "common.h"
#include "vmath.h"
#include "system.h"

class Shader;
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

typedef map<string, Shader*> ShaderMap;
typedef map<string, Texture*> TextureMap;

class Video : public System<Video>, public NoCopy
{
public:
    Video();
    ~Video();

    void init();
    void unloadTextures();

    void renderCube() const;
    void renderFace(const Face& face) const;
    void renderSphere(float radius = 1.0f) const;
    void renderSphereHiQ(float radius = 1.0f) const;
    void renderCylinder(float radius, float height) const;
    void renderCone(float radius, float height) const;
    void renderAxes(float size = 5.0f) const;
    void renderRoundRect(const Vector& lower, const Vector& upper, float r) const;
    void renderSimpleShadow(float r, const Vector& pos, const Collision* level, const Vector& color) const;

    void begin() const;
    void begin(const Matrix& matrix) const;
    void end() const;

    void begin(const Shader* shader) const;
    void end(const Shader* shader) const;

    void bind(const Material* material) const;

    IntPair getResolution() const;
    const IntPairVector& getModes() const;

    unsigned int newList();
    Texture* loadTexture(const string& name, bool mipmap = true);
    Shader*  loadShader(const string& vp, const string& fp);

    bool m_haveShaders;
    bool m_haveShadows;
    bool m_haveShadowsFB;

    bool m_haveVBO;

    bool m_shadowMap3ndPass;

private:
    void loadExtensions();

    GLUquadric*   m_quadric;
    GLUquadric*   m_quadricTex;

    ShaderMap     m_shaders;
    TextureMap    m_textures;
    IntPair       m_resolution;    
    UIntSet       m_lists;

    unsigned int  m_cubeList;

    vector<float> m_circleSin;
    vector<float> m_circleCos;

    mutable const Material* m_lastBound;
};

#endif
