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

struct UV
{
    UV(float u = 0.0f, float v = 0.0f) : u(u), v(v) {}

    union 
    {
        struct
        {
            float u, v;
        };
        float uv[2];
    };
};

struct Face
{
    vector<Vector> vertexes;
    vector<UV>     uv;
    Vector         normal;
};

typedef map<string, Shader*> ShaderMap;
typedef map<string, Texture*> TextureMap;

class Video : public System<Video>, NoCopy
{
public:
    Video();
    ~Video();

    void renderCube() const;
    void renderFace(const Face& face) const;
    void renderSphere(float radius = 1.0f) const;
    void renderWireSphere(float radius = 1.0f) const;
    void renderAxes(float size = 5.0f) const;

    void begin() const;
    void begin(const Matrix& matrix) const;
    void end() const;

    void begin(const Shader* shader) const;
    void end(const Shader* shader) const;

    void enableMaterial(const Material* material) const;
    void disableMaterial(const Material* material) const;

    Texture* loadTexture(const string& name);
    Texture* loadCubeMap(const string& name);
    Shader*  loadShader(const string& vp, const string& fp);

    static PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;

/*
    static PFNGLGENPROGRAMSARBPROC glGenProgramsARB;
    static PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;
    static PFNGLGETPROGRAMIVARBPROC glGetProgramivARB;
    static PFNGLDELETEPROGRAMPROC glDeleteProgramsARB;
    static PFNGLBINDPROGRAMARBPROC glBindProgramARB;
*/
    static PFNGLCREATESHADEROBJECTARBPROC     glCreateShaderObjectARB;
    static PFNGLSHADERSOURCEARBPROC           glShaderSourceARB;
    static PFNGLCOMPILESHADERARBPROC          glCompileShaderARB;

    static PFNGLCREATEPROGRAMOBJECTARBPROC    glCreateProgramObjectARB;
    static PFNGLATTACHOBJECTARBPROC           glAttachObjectARB;
    static PFNGLLINKPROGRAMARBPROC            glLinkProgramARB;
    static PFNGLUSEPROGRAMOBJECTARBPROC       glUseProgramObjectARB;

    static PFNGLGETOBJECTPARAMETERIVARBPROC   glGetObjectParameterivARB;
    static PFNGLGETINFOLOGARBPROC             glGetInfoLogARB;

    static PFNGLDETACHOBJECTARBPROC           glDetachObjectARB;
    static PFNGLDELETEOBJECTARBPROC           glDeleteObjectARB;

    static PFNGLGETUNIFORMLOCATIONARBPROC     glGetUniformLocationARB;
    static PFNGLUNIFORM1IARBPROC              glUniform1iARB;
    static PFNGLUNIFORM3FARBPROC              glUniform3fARB;
    static PFNGLUNIFORMMATRIX4FVARBPROC       glUniformMatrix4fvARB;
    static PFNGLVERTEXATTRIB2FARBPROC         glVertexAttrib2fARB;
    static PFNGLVERTEXATTRIB3FVARBPROC        glVertexAttrib3fvARB;

    bool          m_haveShaders;

private:
    template <typename T>
    void loadProcAddress(const char* name, T& proc) const;

    void loadExtensions();

    GLUquadric*   m_quadricSphere;
    GLUquadric*   m_quadricWireSphere;
    GLUquadric*   m_quadricAxes;

    ShaderMap     m_shaders;
    TextureMap    m_textures;
};

#endif
