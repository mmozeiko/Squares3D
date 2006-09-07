#include <GL/glfw.h>

#include "video.h"
#include "config.h"
#include "file.h"
#include "shader.h"
#include "level.h"
#include "material.h"
#include "texture.h"

static void GLFWCALL sizeCb(int width, int height)
{
    if (width==0 || height==0) return;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // TODO : fix fov
    gluPerspective(45.0, static_cast<float>(width)/height, 0.1, 102.4);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

Video* System<Video>::instance = NULL;

Video::Video() : m_haveShaders(false)
{
    clog << "Initializing video." << endl;

    if (glfwInit() != GL_TRUE)
    {
        throw Exception("glfwInit failed");
    }

    int width = Config::instance->m_video.width;;
    int height = Config::instance->m_video.height;
    bool vsync = Config::instance->m_video.vsync;
    int mode = (Config::instance->m_video.fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW);
    bool systemKeys = Config::instance->m_misc.system_keys;

    int     modes[]  = { mode, GLFW_WINDOW };
    int     depths[] = { 24, 16 };
    IntPair sizes[]  = { make_pair(width, height), make_pair(800, 600) };

    bool fullscr, success = false;

    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, 1);
    if (Config::instance->m_video.samples > 0)
    {
        glfwOpenWindowHint(GLFW_FSAA_SAMPLES, Config::instance->m_video.samples);
    }

    for (int m=0; m<sizeOfArray(modes) && !success; m++)
    {
        for (int d=0; d<sizeOfArray(depths) && !success; d++)
        {
            for (int s=0; s<sizeOfArray(sizes) && !success; s++)
            {
                success = glfwOpenWindow(
                    sizes[s].first,
                    sizes[s].second,
                    8, 8, 8, 0,
                    depths[d], 0,
                    modes[m]) == GL_TRUE;

                fullscr = (modes[m]==GLFW_FULLSCREEN);
            }
        }
    }

    if (!success)
    {
        throw Exception("glfwOpenWindow failed!");
    }

    clog << " * Version  : " << glGetString(GL_VERSION) << endl
         << " * Vendor   : " << glGetString(GL_VENDOR) << endl
         << " * Renderer : " << glGetString(GL_RENDERER) << endl;

    glfwSetWindowSizeCallback(sizeCb);

    glfwSetWindowTitle("Squares 3D");
    glfwSwapInterval(vsync ? 1 : 0);

    glfwGetWindowSize(&width, &height);
    if (fullscr)
    {
        if (!systemKeys)
        {
            glfwDisable(GLFW_SYSTEM_KEYS);
        }
    }
    else
    {
        GLFWvidmode desktop;
        glfwGetDesktopMode(&desktop);
        glfwSetWindowPos((desktop.Width-width)/2, (desktop.Height-height)/2);
    }

    glfwDisable(GLFW_AUTO_POLL_EVENTS);
    glfwDisable(GLFW_KEY_REPEAT);
    glfwDisable(GLFW_MOUSE_CURSOR);

    loadExtensions();
    m_quadricSphere = gluNewQuadric();
    m_quadricAxes = gluNewQuadric();
    m_quadricWireSphere = gluNewQuadric();
    gluQuadricTexture(m_quadricSphere, GLU_TRUE);
    gluQuadricNormals(m_quadricSphere, GLU_TRUE);
    gluQuadricDrawStyle(m_quadricWireSphere, GLU_SILHOUETTE);
}

Video::~Video()
{
    clog << "Closing video." << endl;

    gluDeleteQuadric(m_quadricSphere);
    gluDeleteQuadric(m_quadricAxes);
    gluDeleteQuadric(m_quadricWireSphere);

    for each_(TextureMap, m_textures, iter)
    {
        delete iter->second;
    }

    for each_(ShaderMap, m_shaders, iter)
    {
        delete iter->second;
    }

    glfwCloseWindow();
    glfwTerminate();
}

void Video::renderCube() const
{
    static bool first = true;
    static unsigned int list;

    if (first)
    {
        list = glGenLists(1);
        glNewList(list, GL_COMPILE);

        // -0.5 .. 0.5
        static const Vector vertices[] = {
            /* 0 */ Vector(-0.5, -0.5, -0.5),
            /* 1 */ Vector( 0.5, -0.5, -0.5),
            /* 2 */ Vector( 0.5, -0.5,  0.5),
            /* 3 */ Vector(-0.5, -0.5,  0.5),

            /* 4 */ Vector(-0.5, 0.5, -0.5),
            /* 5 */ Vector( 0.5, 0.5, -0.5),
            /* 6 */ Vector( 0.5, 0.5,  0.5),
            /* 7 */ Vector(-0.5, 0.5,  0.5),
        };

        static const int faces[][4] = {
            { 0, 1, 2, 3 }, // bottom
            { 4, 7, 6, 5 }, // up
            { 4, 5, 1, 0 }, // front
            { 6, 7, 3, 2 }, // back
            { 7, 4, 0, 3 }, // left
            { 5, 6, 2, 1 }, // right
        };
        
        static const Vector normals[] = {
            Vector(0.0, -1.0, 0.0), // bottom
            Vector(0.0,  1.0, 0.0), // up
            Vector(0.0, 0.0, -1.0), // front
            Vector(0.0, 0.0,  1.0), // back
            Vector(-1.0, 0.0, 0.0), // left
            Vector( 1.0, 0.0, 0.0), // right
        };

        static const UV uv[] = {
            UV(1.0, 0.0),
            UV(0.0, 0.0),
            UV(0.0, 1.0),
            UV(1.0, 1.0),
        };

        Face f;
        f.vertexes.resize(4);
        f.uv.resize(4);


        for (size_t i = 0; i < sizeOfArray(faces); i++)
        {
            f.normal = normals[i];
            for (int k=0; k<4; k++)
            {
                f.vertexes[k] = vertices[faces[i][k]];
                f.uv[k] = uv[k];
            }
            renderFace(f);
        }

        glEndList();

        first = false;
    }

    glCallList(list);
}

void Video::renderFace(const Face& face) const
{
    glBegin(GL_POLYGON);
    glNormal3fv(face.normal.v);
    for (size_t i = 0; i < face.vertexes.size(); i++)
    {
        glTexCoord2fv(face.uv[i].uv);
        glVertex3fv(face.vertexes[i].v);
    }
	glEnd();

/*
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex3fv(face.vertexes[0].v);
    glColor3f(1, 1, 1);
    glVertex3fv((face.vertexes[0] + face.normal).v);
    glEnd();
    glEnable(GL_LIGHTING);
*/
}

void Video::renderSphere(float radius) const
{
    gluSphere(m_quadricSphere, radius, 16, 16);
}
    
void Video::renderWireSphere(float radius) const
{
    gluSphere(m_quadricWireSphere, radius, 8, 8);
}
  
void Video::renderAxes(float size) const
{
    static const float red[] = {1.0, 0.0, 0.0};
    static const float green[] = {0.0, 1.0, 0.0};
    static const float blue[] = {0.0, 0.0, 1.0};

    static bool first = true;
    static unsigned int list;
    
    if (first)
    {
        list = glGenLists(1);
        glNewList(list, GL_COMPILE);

        glDisable(GL_LIGHTING);

        glBegin(GL_LINES);
        glColor3fv(red);
        glVertex3f(-size, 0.0, 0.0);
        glVertex3f(size, 0.0, 0.0);

        glColor3fv(green);
        glVertex3f(0.0, -size, 0.0);
        glVertex3f(0.0, size, 0.0);

        glColor3fv(blue);
        glVertex3f(0.0, 0.0, -size);
        glVertex3f(0.0, 0.0, size);
        glEnd();

        glEnable(GL_LIGHTING);

        glPushMatrix();
        glColor3fv(red);
        glTranslatef(size, 0.0, 0.0);
        glRotatef(90.0, 0.0, 1.0, 0.0);
        gluCylinder(m_quadricAxes, 0.2, 0.0, 1.0, 32, 32);
        glRotatef(180.0, 0.0, 1.0, 0.0);
        gluDisk(m_quadricAxes, 0.0, 0.2, 32, 32);
        glPopMatrix();

        glPushMatrix();
        glColor3fv(green);
        glTranslatef(0.0, size, 0.0);
        glRotatef(-90.0, 1.0, 0.0, 0.0);
        gluCylinder(m_quadricAxes, 0.2, 0.0, 1.0, 32, 32);
        glRotatef(180.0, 0.0, 1.0, 0.0);
        gluDisk(m_quadricAxes, 0.0, 0.2, 32, 32);
        glPopMatrix();

        glPushMatrix();
        glColor3fv(blue);
        glTranslatef(0.0, 0.0, size);
        gluCylinder(m_quadricAxes, 0.2, 0.0, 1.0, 32, 32);
        glRotatef(180.0, 0.0, 1.0, 0.0);
        gluDisk(m_quadricAxes, 0.0, 0.2, 32, 32);
        glPopMatrix();

        glEndList();
        first = false;
    }

    glCallList(list);
}

void Video::begin() const
{
    glPushMatrix();
}

void Video::begin(const Matrix& matrix) const
{
    glPushMatrix();
    glMultMatrixf(matrix.m);
}

void Video::end() const
{
    glPopMatrix();
}


void Video::begin(const Shader* shader) const
{
    if (shader != NULL)
    {
        shader->begin();
    }
}

void Video::end(const Shader* shader) const
{
    if (shader != NULL)
    {
        shader->end();
    }
}

void Video::enableMaterial(const Material* material) const
{
    if (material != NULL)
    {
        material->enable();
    }
}

void Video::disableMaterial(const Material* material) const
{
    if (material != NULL)
    {
        material->disable();
    }
}

Texture* Video::loadTexture(const string& name)
{
    TextureMap::iterator iter = m_textures.find(name);
    if (iter != m_textures.end())
    {
        return iter->second;
    }
    
    Texture* texture = new Texture2D(name);
    return m_textures.insert(make_pair(name, texture)).first->second;
}

Texture* Video::loadCubeMap(const string& name)
{
    TextureMap::iterator iter = m_textures.find(name);
    if (iter != m_textures.end())
    {
        return iter->second;
    }
    
    Texture* texture = new TextureCube(name);
    return m_textures.insert(make_pair(name, texture)).first->second;
}

Shader* Video::loadShader(const string& vp, const string& fp)
{
    if (m_haveShaders)
    {
        ShaderMap::const_iterator iter = m_shaders.find(vp+"::"+fp);
        if (iter != m_shaders.end())
        {
            return iter->second;
        }

        string vprogram, fprogram;

        string vp_filename = "/data/shaders/" + vp;
        string fp_filename = "/data/shaders/" + fp;

        File::Reader file(vp_filename);
        if (!file.is_open())
        {
            throw Exception("Shader '" + vp + "' not found");
        }
        vector<char> v(file.size());
        file.read(&v[0], file.size());
        file.close();
        vprogram.assign(v.begin(), v.end());

        file.open(fp_filename);
        if (!file.is_open())
        {
            throw Exception("Shader '" + fp + "' not found");
        }
        v.resize(file.size());
        file.read(&v[0], file.size());
        file.close();
        fprogram.assign(v.begin(), v.end());


        Shader* shader = new Shader(vprogram, fprogram);
        return m_shaders.insert(make_pair(vp+"::"+fp, shader)).first->second;
    }

    return NULL;
    //throw Exception("Shaders not supported, GENA HAUZE!");
}

PFNGLACTIVETEXTUREARBPROC   Video::glActiveTextureARB = NULL;

/*
PFNGLGENPROGRAMSARBPROC     Video::glGenProgramsARB = NULL;
PFNGLPROGRAMSTRINGARBPROC   Video::glProgramStringARB = NULL;
PFNGLGETPROGRAMIVARBPROC    Video::glGetProgramivARB = NULL;
PFNGLDELETEPROGRAMPROC      Video::glDeleteProgramsARB = NULL;
PFNGLBINDPROGRAMARBPROC     Video::glBindProgramARB = NULL;
*/

PFNGLCREATESHADEROBJECTARBPROC     Video::glCreateShaderObjectARB;
PFNGLSHADERSOURCEARBPROC           Video::glShaderSourceARB;
PFNGLCOMPILESHADERARBPROC          Video::glCompileShaderARB;

PFNGLCREATEPROGRAMOBJECTARBPROC    Video::glCreateProgramObjectARB;
PFNGLATTACHOBJECTARBPROC           Video::glAttachObjectARB;
PFNGLLINKPROGRAMARBPROC            Video::glLinkProgramARB;
PFNGLUSEPROGRAMOBJECTARBPROC       Video::glUseProgramObjectARB;

PFNGLGETOBJECTPARAMETERIVARBPROC   Video::glGetObjectParameterivARB;
PFNGLGETINFOLOGARBPROC             Video::glGetInfoLogARB;

PFNGLDETACHOBJECTARBPROC           Video::glDetachObjectARB;
PFNGLDELETEOBJECTARBPROC           Video::glDeleteObjectARB;

PFNGLGETUNIFORMLOCATIONARBPROC     Video::glGetUniformLocationARB;
PFNGLUNIFORM1IARBPROC              Video::glUniform1iARB;
PFNGLUNIFORM3FARBPROC              Video::glUniform3fARB;
PFNGLUNIFORMMATRIX4FVARBPROC       Video::glUniformMatrix4fvARB;
PFNGLVERTEXATTRIB2FARBPROC         Video::glVertexAttrib2fARB;
PFNGLVERTEXATTRIB3FVARBPROC        Video::glVertexAttrib3fvARB;

template <typename T>
void Video::loadProcAddress(const char* name, T& proc) const
{
    proc = reinterpret_cast<T>(glfwGetProcAddress(name));
    if (proc == NULL)
    {
        throw Exception("Address of '" + string(name) + "' not found");
    }
}

#define loadProc(X) loadProcAddress(#X, X)

void Video::loadExtensions()
{
    int major=0, minor=0;
    glfwGetGLVersion(&major, &minor, NULL);
    if (major<2 && minor<5)
    {
        static const char* needed[] = {
            "GL_ARB_multitexture",
            "GL_ARB_texture_cube_map",
        };
        for (size_t i=0; i<sizeOfArray(needed); i++)
        {
            if (glfwExtensionSupported(needed[i])==GL_FALSE)
            {
                throw Exception("Needed OpenGL extension '" + string(needed[i]) + "' not supported");
            }
        }
    }

    loadProc(glActiveTextureARB);

/*
if (glfwExtensionSupported("GL_ARB_fragment_program") && 
        glfwExtensionSupported("GL_ARB_vertex_program"))
    {
        m_haveShaders = true;
        loadProc(glGenProgramsARB);
        loadProc(glProgramStringARB);
        loadProc(glGetProgramivARB);
        loadProc(glDeleteProgramsARB);
        loadProc(glBindProgramARB);
    }
*/
    if (glfwExtensionSupported("GL_ARB_fragment_shader") && 
        glfwExtensionSupported("GL_ARB_vertex_shader") &&
        Config::instance->m_video.useShaders)
    {
        m_haveShaders = true;

        loadProc(glCreateShaderObjectARB);
        loadProc(glShaderSourceARB);
        loadProc(glCompileShaderARB);

        loadProc(glCreateProgramObjectARB);
        loadProc(glAttachObjectARB);
        loadProc(glLinkProgramARB);
        loadProc(glUseProgramObjectARB);

        loadProc(glGetObjectParameterivARB);
        loadProc(glGetInfoLogARB);

        loadProc(glDetachObjectARB);
        loadProc(glDeleteObjectARB);

        loadProc(glGetUniformLocationARB);
        loadProc(glUniform1iARB);
        loadProc(glUniform3fARB);
        loadProc(glUniformMatrix4fvARB);
        
        loadProc(glVertexAttrib2fARB);
        loadProc(glVertexAttrib3fvARB);
    }
}
