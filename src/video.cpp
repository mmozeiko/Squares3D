#include "video.h"
#include "game.h"
#include "config.h"
#include "file.h"
#include "shader.h"

#include <GL/glfw.h>

static void GLFWCALL sizeCb(int width, int height)
{
  if (width==0 || height==0) return;
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // TODO : fix fov
  gluPerspective(45.0, ((double)width) / ((double)height), 0.1, 102.4);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

Video::Video(const Game* game) : m_config(game->m_config.get())
{
    clog << "Initializing video." << endl;

    if (glfwInit() != GL_TRUE)
    {
        throw Exception("glfwInit failed");
    }

    int width = m_config->m_video.width;;
    int height = m_config->m_video.height;
    bool vsync = m_config->m_video.vsync;
    int mode = (m_config->m_video.fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW);
    bool systemKeys = m_config->m_misc.system_keys;

    int     modes[]  = { mode, GLFW_WINDOW };
    int     depths[] = { 32, 24, 16 };
    IntPair sizes[]  = { make_pair(width, height), make_pair(800, 600) };

    bool fullscr, success = false;

    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, 1);

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
}

Video::~Video()
{
    clog << "Closing video." << endl;

    for each_(UIntMap, m_textures, iter)
    {
        glDeleteTextures(1, &iter->second);
    }

    glfwCloseWindow();
    glfwTerminate();
}

void Video::renderCube() const
{
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

    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex3fv(face.vertexes[0].v);
    glColor3f(1, 1, 1);
    glVertex3fv((face.vertexes[0] + face.normal).v);
    glEnd();
    glEnable(GL_LIGHTING);
}

void Video::renderSphere(float radius) const
{
    GLUquadric* q =  gluNewQuadric();
    gluQuadricTexture(q, GLU_TRUE);
    gluQuadricNormals(q, GLU_TRUE);
    gluSphere(q, radius, 32, 32);
    gluDeleteQuadric(q);
}
    
void Video::renderWireSphere(float radius) const
{
    GLUquadric* q =  gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_SILHOUETTE);
    gluSphere(q, radius, 32, 32);
    gluDeleteQuadric(q);
}
  
void Video::renderAxes(float size) const
{
    const float red[] = {1.0, 0.0, 0.0};
    const float green[] = {0.0, 1.0, 0.0};
    const float blue[] = {0.0, 0.0, 1.0};

    
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
    
    GLUquadric* q =  gluNewQuadric();
    
    glPushMatrix();
    glColor3fv(red);
    glTranslatef(size, 0.0, 0.0);
    glRotatef(90.0, 0.0, 1.0, 0.0);
    gluCylinder(q, 0.2, 0.0, 1.0, 32, 32);
    glRotatef(180.0, 0.0, 1.0, 0.0);
    gluDisk(q, 0.0, 0.2, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glColor3fv(green);
    glTranslatef(0.0, size, 0.0);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    gluCylinder(q, 0.2, 0.0, 1.0, 32, 32);
    glRotatef(180.0, 0.0, 1.0, 0.0);
    gluDisk(q, 0.0, 0.2, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glColor3fv(blue);
    glTranslatef(0.0, 0.0, size);
    gluCylinder(q, 0.2, 0.0, 1.0, 32, 32);
    glRotatef(180.0, 0.0, 1.0, 0.0);
    gluDisk(q, 0.0, 0.2, 32, 32);
    glPopMatrix();

    gluDeleteQuadric(q);
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

void Video::applyTexture(unsigned int texture) const
{
    glBindTexture(GL_TEXTURE_2D, texture);
}

unsigned int Video::loadTexture(const string& name)
{
    UIntMap::const_iterator iter = m_textures.find(name);
    if (iter != m_textures.end())
    {
        return iter->second;
    }

    string filename = "/data/textures/" + name;
    File::Reader file(filename);
    if (!file.is_open())
    {
        throw Exception("Texture '" + name + "' not found");
    }
    unsigned int filesize = file.filesize();

    vector<char> data(filesize);
    file.read(&data[0], filesize);
    file.close();

    glEnable(GL_TEXTURE_2D);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    if (glfwLoadMemoryTexture2D(static_cast<void*>(&data[0]), filesize, GLFW_BUILD_MIPMAPS_BIT) == GL_FALSE)
    {
        throw Exception("Texture '" + name + "' failed to load");
    }
    glDisable(GL_TEXTURE_2D);
    
    m_textures.insert(make_pair(name, texture));
    return texture;
}

Shader* Video::loadShader(const string& vp, const string& fp)
{
    ShaderMap::const_iterator iter = m_shaders.find(vp+"::"+fp);
    if (iter != m_shaders.end())
    {
        return iter->second;
    }

    if (glfwExtensionSupported("GL_ARB_multitexture") &&
        glfwExtensionSupported("GL_ARB_fragment_program") && 
        glfwExtensionSupported("GL_ARB_vertex_program"))
    {
        string vprogram, fprogram;

        string vp_filename = "/data/shaders/" + vp;
        string fp_filename = "/data/shaders/" + fp;

        File::Reader file(vp_filename);
        if (!file.is_open())
        {
            throw Exception("Shader '" + vp + "' not found");
        }
        vector<char> v(file.filesize());
        file.read(&v[0], file.filesize());
        file.close();
        vprogram.assign(v.begin(), v.end());

        file.open(fp_filename);
        if (!file.is_open())
        {
            throw Exception("Shader '" + fp + "' not found");
        }
        v.resize(file.filesize());
        file.read(&v[0], file.filesize());
        file.close();
        fprogram.assign(v.begin(), v.end());


        Shader* shader = new Shader(vprogram, fprogram);
        m_shaders.insert(make_pair(vp+"::"+fp, shader));
        return shader;
    }

    throw Exception("Shaders not supported, sorry!");
}

PFNGLACTIVETEXTUREARBPROC   Video::glActiveTextureARB = NULL;

PFNGLGENPROGRAMSARBPROC     Video::glGenProgramsARB = NULL;
PFNGLPROGRAMSTRINGARBPROC   Video::glProgramStringARB = NULL;
PFNGLGETPROGRAMIVARBPROC    Video::glGetProgramivARB = NULL;
PFNGLDELETEPROGRAMPROC      Video::glDeleteProgramsARB = NULL;
PFNGLBINDPROGRAMARBPROC     Video::glBindProgramARB = NULL;

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
    if (glfwExtensionSupported("GL_ARB_multitexture"))
    {
        loadProc(glActiveTextureARB);
    }

    if (glfwExtensionSupported("GL_ARB_fragment_program") && 
        glfwExtensionSupported("GL_ARB_vertex_program"))
    {
        loadProc(glGenProgramsARB);
        loadProc(glProgramStringARB);
        loadProc(glGetProgramivARB);
        loadProc(glDeleteProgramsARB);
        loadProc(glBindProgramARB);
    }
}
