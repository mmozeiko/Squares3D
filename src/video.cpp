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
    gluPerspective(45.0, static_cast<float>(width)/static_cast<float>(height), 0.1, 102.4);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

Video* System<Video>::instance = NULL;

Video::Video() : m_haveShaders(false), m_haveShadows(false), m_haveShadowsFB(false), m_haveVBO(false)
{
    setInstance(this);

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
                    8, 8, 8, 8,
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

    clog << " * Version     : " << glGetString(GL_VERSION) << endl
         << " * Vendor      : " << glGetString(GL_VENDOR) << endl
         << " * Renderer    : " << glGetString(GL_RENDERER) << endl;
    clog << " * Video modes : ";
    IntPairVector m = getModes();
    for each_const(IntPairVector, m, i)
    {
        clog << i->first << 'x' << i->second << ' ';
    }
    clog << endl;

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

    Config::instance->m_video.samples = glfwGetWindowParam(GLFW_FSAA_SAMPLES);

    glfwDisable(GLFW_KEY_REPEAT);
    glfwDisable(GLFW_MOUSE_CURSOR);

    loadExtensions();
    m_quadricSphere = gluNewQuadric();
    m_quadricAxes = gluNewQuadric();
    m_quadricSphereHiQ = gluNewQuadric();
    gluQuadricTexture(m_quadricSphere, GLU_TRUE);
    gluQuadricNormals(m_quadricSphere, GLU_TRUE);
    gluQuadricTexture(m_quadricSphereHiQ, GLU_TRUE);
    gluQuadricNormals(m_quadricSphereHiQ, GLU_TRUE);
    m_resolution = make_pair(width, height);
}

Video::~Video()
{
    clog << "Closing video." << endl;

    gluDeleteQuadric(m_quadricSphere);
    gluDeleteQuadric(m_quadricAxes);
    gluDeleteQuadric(m_quadricSphereHiQ);

    unloadTextures();

    for each_(ShaderMap, m_shaders, iter)
    {
        delete iter->second;
    }
    m_shaders.clear();

    for each_(UIntSet, m_lists, iter)
    {
        glDeleteLists(*iter, 1);
    }
    m_lists.clear();

    glfwCloseWindow();
    glfwTerminate();
}

void Video::init()
{
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_NORMALIZE);

    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY); 
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
    glEnableClientState(GL_NORMAL_ARRAY); 

    // -0.5 .. 0.5
    static const float vertices[][3] = {
        /* 0 */ { -0.5, -0.5, -0.5 },
        /* 1 */ {  0.5, -0.5, -0.5 },
        /* 2 */ {  0.5, -0.5,  0.5 },
        /* 3 */ { -0.5, -0.5,  0.5 },

        /* 4 */ { -0.5,  0.5, -0.5 },
        /* 5 */ {  0.5,  0.5, -0.5 },
        /* 6 */ {  0.5,  0.5,  0.5 },
        /* 7 */ { -0.5,  0.5,  0.5 },
    };

    static const int faces[][4] = {
        { 0, 1, 3, 2 }, // bottom
        { 4, 7, 5, 6 }, // up
        { 4, 5, 0, 1 }, // front
        { 6, 7, 2, 3 }, // back
        { 7, 4, 3, 0 }, // left
        { 5, 6, 1, 2 }, // right
    };
    
    static const float normals[][3] = {
        {  0.0, -1.0,  0.0 }, // bottom
        {  0.0,  1.0,  0.0 }, // up
        {  0.0,  0.0, -1.0 }, // front
        {  0.0,  0.0,  1.0 }, // back
        { -1.0,  0.0,  0.0 }, // left
        {  1.0,  0.0,  0.0 }, // right
    };

    static const float uv[][2] = {
        { 1.0, 0.0 },
        { 0.0, 0.0 },
        { 1.0, 1.0 },
        { 0.0, 1.0 },
    };

    m_cubeList = Video::instance->newList();
    glNewList(m_cubeList, GL_COMPILE);

    for (size_t i = 0; i < sizeOfArray(faces); i++)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for (int k=0; k<4; k++)
        {
            glTexCoord2fv(uv[k]);
            glNormal3fv(normals[i]);
            glVertex3fv(vertices[faces[i][k]]);
        }
        glEnd();
    }

    glEndList();
}

void Video::renderCube() const
{
    glCallList(m_cubeList);
}

void Video::renderFace(const Face& face) const
{
    glTexCoordPointer(2, GL_FLOAT, sizeof(UV), &face.uv[0]);
    glVertexPointer(3, GL_FLOAT, sizeof(Vector), &face.vertexes[0]);
    if (face.normal.size() > 0)
    {
        glNormalPointer(GL_FLOAT, sizeof(Vector), &face.normal[0]);
    }
    glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(face.uv.size()));

/*    glBegin(GL_TRIANGLE_STRIP);
    glNormal3fv(face.normal.v);
    for (size_t i = 0; i < face.vertexes.size(); i++)
    {
        glTexCoord2fv(face.uv[i].uv);
        glVertex3fv(face.vertexes[i].v);
    }
    glEnd();
*/

/*  normal rendering:
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
    
void Video::renderSphereHiQ(float radius) const
{
    gluSphere(m_quadricSphereHiQ, radius, 64, 64);
}
  
void Video::renderAxes(float size) const
{
    static const float red[] = {1.0, 0.0, 0.0};
    static const float green[] = {0.0, 1.0, 0.0};
    static const float blue[] = {0.0, 0.0, 1.0};

    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);
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

    glPopAttrib();
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

Texture* Video::loadTexture(const string& name, bool mipmap)
{
    TextureMap::iterator iter = m_textures.find(name);
    if (iter != m_textures.end())
    {
        return iter->second;
    }
    
    Texture* texture = new Texture(name, mipmap);
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

PFNGLACTIVETEXTUREARBPROC           Video::glActiveTextureARB = NULL;

/*
PFNGLGENPROGRAMSARBPROC     Video::glGenProgramsARB = NULL;
PFNGLPROGRAMSTRINGARBPROC   Video::glProgramStringARB = NULL;
PFNGLGETPROGRAMIVARBPROC    Video::glGetProgramivARB = NULL;
PFNGLDELETEPROGRAMPROC      Video::glDeleteProgramsARB = NULL;
PFNGLBINDPROGRAMARBPROC     Video::glBindProgramARB = NULL;
*/

PFNGLCREATESHADEROBJECTARBPROC      Video::glCreateShaderObjectARB = NULL;
PFNGLSHADERSOURCEARBPROC            Video::glShaderSourceARB = NULL;
PFNGLCOMPILESHADERARBPROC           Video::glCompileShaderARB = NULL;

PFNGLCREATEPROGRAMOBJECTARBPROC     Video::glCreateProgramObjectARB = NULL;
PFNGLATTACHOBJECTARBPROC            Video::glAttachObjectARB = NULL;
PFNGLLINKPROGRAMARBPROC             Video::glLinkProgramARB = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC        Video::glUseProgramObjectARB = NULL;

PFNGLGETOBJECTPARAMETERIVARBPROC    Video::glGetObjectParameterivARB = NULL;
PFNGLGETINFOLOGARBPROC              Video::glGetInfoLogARB = NULL;

PFNGLDETACHOBJECTARBPROC            Video::glDetachObjectARB = NULL;
PFNGLDELETEOBJECTARBPROC            Video::glDeleteObjectARB = NULL;

PFNGLGETUNIFORMLOCATIONARBPROC      Video::glGetUniformLocationARB = NULL;
PFNGLUNIFORM1IARBPROC               Video::glUniform1iARB = NULL;
PFNGLUNIFORM3FARBPROC               Video::glUniform3fARB = NULL;
PFNGLUNIFORMMATRIX4FVARBPROC        Video::glUniformMatrix4fvARB = NULL;
PFNGLVERTEXATTRIB2FARBPROC          Video::glVertexAttrib2fARB = NULL;
PFNGLVERTEXATTRIB3FVARBPROC         Video::glVertexAttrib3fvARB = NULL;

PFNGLGENFRAMEBUFFERSEXTPROC         Video::glGenFramebuffersEXT = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC         Video::glBindFramebufferEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    Video::glFramebufferTexture2DEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  Video::glCheckFramebufferStatusEXT = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC      Video::glDeleteFramebuffersEXT = NULL;

PFNGLGENRENDERBUFFERSEXTPROC        Video::glGenRenderbuffersEXT = NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC     Video::glRenderbufferStorageEXT = NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC Video::glFramebufferRenderbufferEXT = NULL;
PFNGLDELETERENDERBUFFERSEXTPROC     Video::glDeleteRenderbuffersEXT = NULL;
PFNGLBINDRENDERBUFFEREXTPROC        Video::glBindRenderbufferEXT = NULL;

PFNGLGENBUFFERSARBPROC              Video::glGenBuffersARB = NULL;
PFNGLBINDBUFFERARBPROC              Video::glBindBufferARB = NULL;
PFNGLBUFFERDATAARBPROC              Video::glBufferDataARB = NULL;
PFNGLDELETEBUFFERSARBPROC           Video::glDeleteBuffersARB = NULL;
PFNGLBUFFERSUBDATAARBPROC           Video::glBufferSubDataARB = NULL;

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
        glfwExtensionSupported("GL_ARB_vertex_shader"))
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

    if (glfwExtensionSupported("GL_EXT_framebuffer_object"))
    {
        loadProc(glGenFramebuffersEXT);
        loadProc(glBindFramebufferEXT);
        loadProc(glFramebufferTexture2DEXT);
        loadProc(glCheckFramebufferStatusEXT);
        loadProc(glDeleteFramebuffersEXT);

        loadProc(glGenRenderbuffersEXT);
        loadProc(glRenderbufferStorageEXT);
        loadProc(glFramebufferRenderbufferEXT);
        loadProc(glDeleteRenderbuffersEXT);
        loadProc(glBindRenderbufferEXT);
    }

    if (glfwExtensionSupported("GL_ARB_vertex_buffer_object"))
    {
        m_haveVBO = true;

        loadProc(glGenBuffersARB);
        loadProc(glBindBufferARB);
        loadProc(glBufferDataARB);
        loadProc(glDeleteBuffersARB);
        loadProc(glBufferSubDataARB);
    }

    if (glfwExtensionSupported("GL_ARB_depth_texture") &&
        glfwExtensionSupported("GL_ARB_shadow"))
    {
        m_haveShadows = true;
        if (glfwExtensionSupported("GL_EXT_framebuffer_object"))
        {
            m_haveShadowsFB = true;
        }
        else
        {
            Config::instance->m_video.shadowmap_size = 0;
        }
    }

     if (!m_haveShaders)
    {
        Config::instance->m_video.use_shaders = 0;
    }

    if (!m_haveShadows)
    {
        Config::instance->m_video.shadow_type = 0;
    }
}

IntPair Video::getResolution() const
{
    return m_resolution;
}

unsigned int Video::newList()
{
    unsigned int list = glGenLists(1);
    m_lists.insert(list);
    return list;
}

IntPairVector Video::getModes() const
{
    static const int commonWidth[] = { 640, 800, 1024, 1280, 1440, 1600, 1680};
    static const IntSet commonWidthSet(commonWidth, commonWidth + sizeOfArray(commonWidth));

    static IntPairVector modes;
    static bool first = true;

    if (first)
    {
        GLFWvidmode list[200];
        int count = glfwGetVideoModes(list, sizeOfArray(list));
        for (int i=0; i<count; i++)
        {
            float aspect = static_cast<float>(list[i].Width) / static_cast<float>(list[i].Height);
            if ( (aspect == 4.0f/3.0f || aspect == 5.0f/4.0f || aspect == 16.0f/9.0f || aspect == 16.0f/10.0f) &&
                 (list[i].BlueBits + list[i].GreenBits + list[i].RedBits >= 24) &&
                 list[i].Height >= 480 &&
                 foundInSet(commonWidthSet, list[i].Width))
            {
                modes.push_back(make_pair(list[i].Width, list[i].Height));
            }
        }
        first = false;
    }

    return modes;
}

void Video::unloadTextures()
{
    for each_const(TextureMap, m_textures, iter)
    {
        delete iter->second;
    }
    m_textures.clear();
}

void Video::renderRoundRect(const Vector& lower, const Vector& upper, float r) const
{
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_POLYGON);
        for (int i=0; i<9; i++)
        {
            glVertex2f(lower.x - r*std::sinf(i*M_PI_2/8), lower.y - r*std::cosf(i*M_PI_2/8));
        }
        for (int i=0; i<9; i++)
        {
            glVertex2f(lower.x - r*std::sinf((8-i)*M_PI_2/8), upper.y + r*std::cosf((8-i)*M_PI_2/8));
        }
        for (int i=0; i<9; i++)
        {
            glVertex2f(upper.x + r*std::sinf(i*M_PI_2/8), upper.y + r*std::cosf(i*M_PI_2/8));
        }
        for (int i=0; i<9; i++)
        {
            glVertex2f(upper.x + r*std::sinf((8-i)*M_PI_2/8), lower.y - r*std::cosf((8-i)*M_PI_2/8));
        }
    glEnd();
}
