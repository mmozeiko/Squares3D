#include <GL/glfw.h>

#include "video.h"
#include "video_ext.h"
#include "config.h"
#include "file.h"
#include "level.h"
#include "material.h"
#include "texture.h"
#include "geometry.h"
#include "collision.h"
#include "input.h"

static const int CIRCLE_DIVISIONS = 12;

template <class Video> Video* System<Video>::instance = NULL;

static void GLFWCALL sizeCb(int width, int height)
{
    if (width==0 || height==0) return;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, static_cast<float>(width)/static_cast<float>(height), 0.1, 51.2);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void video_setup()
{
    if (glfwInit() != GL_TRUE)
    {
        throw Exception("glfwInit failed");
    }
}

void video_finish()
{
    glfwTerminate();
}

Video::Video() :
    m_haveAnisotropy(false),
    m_maxAnisotropy(0),
    m_haveShadows(false),
    m_haveShadowsFB(false),
    m_haveVBO(false),
    m_shadowMap3ndPass(false),
    m_lastBound(NULL)
{
    setInstance(this);

    clog << "Initializing video." << endl;

#ifndef __APPLE__
    video_setup();
#endif

    int width = Config::instance->m_video.width;;
    int height = Config::instance->m_video.height;
    bool vsync = Config::instance->m_video.vsync;
    int mode = (Config::instance->m_video.fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW);
    bool systemKeys = Config::instance->m_misc.system_keys;

    int     modes[]  = { mode, GLFW_WINDOW };
    int     depths[] = { 24, 16 };
    IntPair sizes[]  = { make_pair(width, height), make_pair(800, 600) };

    bool fullscr = false;
    bool success = false;

    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, 1);
    if (Config::instance->m_video.samples > 0)
    {
        glfwOpenWindowHint(GLFW_FSAA_SAMPLES, Config::instance->m_video.samples);
    }

    for (size_t m=0; m<sizeOfArray(modes) && !success; m++)
    {
        for (size_t d=0; d<sizeOfArray(depths) && !success; d++)
        {
            for (size_t s=0; s<sizeOfArray(sizes) && !success; s++)
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
    const IntPairVector& m = getModes();
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
    if (Config::instance->m_video.samples > 0)
    {
        glEnable(GL_MULTISAMPLE_ARB);
    }

    glfwDisable(GLFW_KEY_REPEAT);
    Input::instance->mouseVisible(false);

    loadExtensions();
 
    m_resolution = make_pair(width, height);

    for (int i=CIRCLE_DIVISIONS; i>=0; i--)
    {
        m_circleSin.push_back(std::sin(i*2.0f*M_PI/CIRCLE_DIVISIONS));
        m_circleCos.push_back(std::cos(i*2.0f*M_PI/CIRCLE_DIVISIONS));
    }
}

Video::~Video()
{
    clog << "Closing video." << endl;

    unloadTextures();

    for each_(UIntSet, m_lists, iter)
    {
        glDeleteLists(*iter, 1);
    }
    m_lists.clear();

    glfwCloseWindow();
#ifndef __APPLE__
    video_finish();
#endif
}

void Video::init()
{
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

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

    m_quadricTexSphere = gluNewQuadric();
    gluQuadricTexture(m_quadricTexSphere, GLU_TRUE);
    gluQuadricNormals(m_quadricTexSphere, GLU_TRUE);
/*

    m_cylinderList = Video::instance->newList();
    glNewList(m_cylinderList, GL_COMPILE);

        GLUquadric* m_quadricTexCylinder = gluNewQuadric();
        gluQuadricTexture(m_quadricTexCylinder, GLU_TRUE);
        gluQuadricNormals(m_quadricTexCylinder, GLU_TRUE);
        GLUquadric* m_quadricTexDisc = gluNewQuadric();
        gluQuadricTexture(m_quadricTexDisc, GLU_TRUE);
        gluQuadricNormals(m_quadricTexDisc, GLU_TRUE);

        gluCylinder(m_quadricTexCylinder, 1.0f, 1.0f, 1.0f, 16, 16);
    
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 1.0f);
        gluDisk(m_quadricTexDisc, 0.0, 1.0f, 16, 16);
        glPopMatrix();

        glPushMatrix();
        glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        gluDisk(m_quadricTexDisc, 0.0, 1.0f, 16, 16);
        glPopMatrix();

        gluDeleteQuadric(m_quadricTexCylinder);
        gluDeleteQuadric(m_quadricTexDisc);
    glEndList();

    m_coneList = Video::instance->newList();
    glNewList(m_coneList, GL_COMPILE);
        GLUquadric* m_quadricTexDisc2 = gluNewQuadric();
        gluQuadricTexture(m_quadricTexDisc2, GLU_TRUE);
        gluQuadricNormals(m_quadricTexDisc2, GLU_TRUE);
        GLUquadric* m_quadricTexCone = gluNewQuadric();
        gluQuadricTexture(m_quadricTexCone, GLU_TRUE);
        gluQuadricNormals(m_quadricTexCone, GLU_TRUE);

        gluCylinder(m_quadricTexCone, 1.0f, 0.0f, 1.0f, 16, 16);
        glPushMatrix();
        glRotatef(180.0, 1.0, 0.0, 0.0);
        gluDisk(m_quadricTexDisc2, 0.0, 1.0f, 16, 16);
        glPopMatrix();

        gluDeleteQuadric(m_quadricTexCone);
        gluDeleteQuadric(m_quadricTexDisc2);

    glEndList();
    */
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

    /*// normal rendering:
    if (face.normal.size() > 0)
    {
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3fv(face.vertexes[0].v);
        glColor3f(1, 1, 1);
        glVertex3fv((face.vertexes[0] + face.normal[0]).v);
        glEnd();
        glEnable(GL_LIGHTING);
    }
    */
}

void Video::renderSphere() const
{
    static const int magic[] = {12, 16, 24};
    const int w = magic[Config::instance->m_video.terrain_detail];
    gluSphere(m_quadricTexSphere, 1.0f, w, w);
}

void Video::renderSphere(float radius) const
{
    static const int magic[] = {12, 16, 24};
    const int w = magic[Config::instance->m_video.terrain_detail];
    glPushMatrix();
    gluSphere(m_quadricTexSphere, radius, 12, 12);
    glPopMatrix();
} 

void Video::renderCylinder(float radius, float height) const
{
    glPushMatrix();
    glScalef(radius, radius, height);
        gluCylinder(m_quadricTexSphere, 1.0f, 1.0f, 1.0f, 8, 8);
    
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 1.0f);
        gluDisk(m_quadricTexSphere, 0.0, 1.0f, 8, 8);
        glPopMatrix();

        glPushMatrix();
        glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        gluDisk(m_quadricTexSphere, 0.0, 1.0f, 8, 8);
        glPopMatrix();
    glPopMatrix();
}

void Video::renderCone(float radius, float height) const
{
    glPushMatrix();
    glScalef(radius, radius, height);
        gluCylinder(m_quadricTexSphere, 1.0f, 0.0f, 1.0f, 8, 8);
        glPushMatrix();
        glRotatef(180.0, 1.0, 0.0, 0.0);
        gluDisk(m_quadricTexSphere, 0.0, 1.0f, 8, 8);
        glPopMatrix();
    glPopMatrix();
}

void Video::renderAxes(float size) const
{
/*    static const float red[] = {1.0, 0.0, 0.0};
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
    gluCylinder(m_quadric, 0.2, 0.0, 1.0, 16, 16);
    glRotatef(180.0, 0.0, 1.0, 0.0);
    gluDisk(m_quadric, 0.0, 0.2, 16, 16);
    glPopMatrix();

    glPushMatrix();
    glColor3fv(green);
    glTranslatef(0.0, size, 0.0);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    gluCylinder(m_quadric, 0.2, 0.0, 1.0, 16, 16);
    glRotatef(180.0, 0.0, 1.0, 0.0);
    gluDisk(m_quadric, 0.0, 0.2, 16, 16);
    glPopMatrix();

    glPushMatrix();
    glColor3fv(blue);
    glTranslatef(0.0, 0.0, size);
    gluCylinder(m_quadric, 0.2, 0.0, 1.0, 16, 16);
    glRotatef(180.0, 0.0, 1.0, 0.0);
    gluDisk(m_quadric, 0.0, 0.2, 16, 16);
    glPopMatrix();

    glPopAttrib();
*/}

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

void Video::bind(const Material* material) const
{
    if (m_lastBound == material)
    {
        return;
    }

    if (material != NULL)
    {
        material->bind();
    }

    m_lastBound = material;
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

void Video::loadExtensions()
{   
    if (glfwExtensionSupported("GL_EXT_rescale_normal"))
    {
        clog << "Video: GL_EXT_rescale_normal supported." << endl;
        glEnable(GL_RESCALE_NORMAL_EXT);
    }
    else
    {
        clog << "Video: GL_EXT_rescale_normal unavailable." << endl;
    }

    bool activeTex = false;
//#ifndef GL_ARB_multitexture
    if (glfwExtensionSupported("GL_ARB_multitexture"))
    {
        activeTex = true;
        loadProc(glActiveTextureARB);
    }
//#else
//    activeTex = true;
//#endif
    if (activeTex)
    {
        clog << "Video: GL_ARB_multitexture supported." << endl;
    }
    else
    {
        clog << "Video: GL_ARB_multitexture unavailable." << endl;
    }

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

        //loadProc(glDetachObjectARB);
        loadProc(glDeleteObjectARB);

        loadProc(glGetUniformLocationARB);
        loadProc(glUniform1iARB);
        loadProc(glUniform1fARB);
        loadProc(glUniform4fvARB);
        //loadProc(glUniformMatrix4fvARB);
        
        //loadProc(glVertexAttrib2fARB);
        //loadProc(glVertexAttrib3fvARB);

        clog << "Video: GL_ARB_fragment_shader and GL_ARB_vertex_shader supported." << endl;
    }
    else
    {
        clog << "Video: GL_ARB_fragment_shader or GL_ARB_vertex_shader unavailable." << endl;
    }
    
//#ifndef GL_EXT_texture_filter_anisotropic
    if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic"))
    {
        m_haveAnisotropy = true;
    }
//#else
//    m_haveAnisotropy = true;
//#endif
    if (m_haveAnisotropy)
    {
        clog << "Video: GL_EXT_texture_filter_anisotropic supported." << endl;
    }
    else
    {
        clog << "Video: GL_EXT_texture_filter_anisotropic unavailable." << endl;
    }

    if (m_haveAnisotropy)
    {
        GLint i;
        glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &i);
        m_maxAnisotropy = 0;
        while (i != 1)
        {
            m_maxAnisotropy++;
            i >>= 1;
        }
        if (Config::instance->m_video.anisotropy > m_maxAnisotropy)
        {
            Config::instance->m_video.anisotropy = m_maxAnisotropy;
        }
    }
    else
    {
        Config::instance->m_video.anisotropy = 0;
    }

//#ifndef GL_EXT_framebuffer_object
    if (activeTex && glfwExtensionSupported("GL_EXT_framebuffer_object"))
    {
        m_haveShadowsFB = true;

        loadProc(glGenFramebuffersEXT);
        loadProc(glBindFramebufferEXT);
        loadProc(glFramebufferTexture2DEXT);
        loadProc(glCheckFramebufferStatusEXT);
        loadProc(glDeleteFramebuffersEXT);
    }
//#else
//    m_haveShadowsFB = true;
//#endif
    if (m_haveShadowsFB)
    {
        clog << "Video: GL_EXT_framebuffer_object supported." << endl;
    }
    else
    {
        clog << "Video: GL_EXT_framebuffer_object unavailable." << endl;
    }

//#ifndef GL_ARB_vertex_buffer_object
    if (glfwExtensionSupported("GL_ARB_vertex_buffer_object"))
    {
        m_haveVBO = true;

        loadProc(glGenBuffersARB);
        loadProc(glBindBufferARB);
        loadProc(glBufferDataARB);
        loadProc(glDeleteBuffersARB);
        loadProc(glBufferSubDataARB);
    }
//#endif
    if (m_haveVBO)
    {
        clog << "Video: GL_ARB_vertex_buffer_object supported." << endl;
    }
    else
    {
        clog << "Video: GL_ARB_vertex_buffer_object unavailable." << endl;
    }

    if (activeTex &&
        glfwExtensionSupported("GL_ARB_depth_texture") &&
        glfwExtensionSupported("GL_ARB_shadow"))
    {
        m_haveShadows = true;
    }

    if (m_haveShadows)
    {
        clog << "Video: GL_ARB_shadow and GL_ARB_depth_texture supported." << endl;
    }
    else
    {
        clog << "Video: GL_ARB_shadow or GL_ARB_depth_texture unavailable." << endl;
    }

    if (!m_haveAnisotropy)
    {
        Config::instance->m_video.anisotropy = 0;
    }

    if (!m_haveShadows)
    {
        Config::instance->m_video.shadow_type = 0;
    }

    if (!m_haveShadowsFB)
    {
        Config::instance->m_video.shadowmap_size = 0;
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

const IntPairVector& Video::getModes() const
{
    static const int commonWidth[] = { 800, 1024, 1280, 1440, 1600, 1680};
    static const IntSet commonWidthSet(commonWidth, commonWidth + sizeOfArray(commonWidth));

    static IntPairVector modes;
    static bool first = true;

    if (first)
    {
        GLFWvidmode list[200];
        int count = glfwGetVideoModes(list, sizeOfArray(list));
        for (int i=0; i<count; i++)
        {
            int bpp = list[i].BlueBits + list[i].GreenBits + list[i].RedBits;

            bool aspectGood = (
            	list[i].Height*4 == list[i].Width*3 ||
            	list[i].Height*5 == list[i].Width*4 ||
            	list[i].Height*16 == list[i].Width*9 ||
            	list[i].Height*16 == list[i].Width*10);

            if ( aspectGood && bpp >= 24 && list[i].Height >= 480 && foundIn(commonWidthSet, list[i].Width))
            {
#ifdef __APPLE__
                if (list[i].Height > 480)
                {
                    modes.push_back(make_pair(list[i].Width, list[i].Height));
                }
#else
                modes.push_back(make_pair(list[i].Width, list[i].Height));
#endif
            }
        }
        if (modes.empty())
        {
            modes.push_back(make_pair(800, 600));
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
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);
        for (int i=0; i<9; i++)
        {
            glVertex2f(lower.x - r*std::sin(i*M_PI_2/8), lower.y - r*std::cos(i*M_PI_2/8));
        }
        for (int i=0; i<9; i++)
        {
            glVertex2f(lower.x - r*std::sin((8-i)*M_PI_2/8), upper.y + r*std::cos((8-i)*M_PI_2/8));
        }
        for (int i=0; i<9; i++)
        {
            glVertex2f(upper.x + r*std::sin(i*M_PI_2/8), upper.y + r*std::cos(i*M_PI_2/8));
        }
        for (int i=0; i<9; i++)
        {
            glVertex2f(upper.x + r*std::sin((8-i)*M_PI_2/8), lower.y - r*std::cos((8-i)*M_PI_2/8));
        }
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

void Video::renderSimpleShadow(float r, const Vector& pos, const Collision* level, const Vector& color) const
{
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, -1.0f);
            
    float y = 0.01f;

    if (isPointInRectangle(pos, Vector(-3, 0, -3), Vector(3, 0, 3)))
    {
        y += 0.005f;
    }

    glPushMatrix();

    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    level->renderTri(pos.x, pos.z);

    glBegin(GL_TRIANGLE_FAN);
    
    glColor4fv(color.v);
    float yy = level->getHeight(pos.x, pos.z) + y;
    glVertex3f(pos.x, yy, pos.z);

    glColor4fv(Vector(color.x, color.y, color.z, color.w-0.1f).v);
    for (int i=0; i<=CIRCLE_DIVISIONS; i++)
    {
        float xx = r * m_circleCos[i];
        float zz = r * m_circleSin[i];
        float yy = level->getHeight(pos.x+xx, pos.z+zz) + y;
        glVertex3f(pos.x + xx, yy, pos.z + zz);
    }

    glEnd();
    glPopAttrib();

    glPopMatrix();

    glDisable(GL_POLYGON_OFFSET_FILL);
}
