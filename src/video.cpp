#include "video.h"
#include "game.h"
#include "config.h"
#include "file.h"

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

Video::Video(const Game* game) : m_game(game)
{
    clog << "Initializing video." << endl;

    if (glfwInit() != GL_TRUE)
    {
        throw Exception("glfwInit failed");
    }

    int width = m_game->m_config->video().width;;
    int height = m_game->m_config->video().height;
    bool vsync = m_game->m_config->video().vsync;
    int mode = (m_game->m_config->video().fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW);
    bool systemKeys = m_game->m_config->misc().system_keys;

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
}

Video::~Video()
{
    clog << "Closing video." << endl;
    glfwCloseWindow();
    glfwTerminate();
}

void Video::renderCube(float size) const
{
    float tmp = 0.5f * size;

	static float vertex[8][3] = {
			{-tmp, -tmp, tmp},   // vertex v0
			{tmp,  -tmp, tmp},   // vertex v1
			{tmp,  -tmp, -tmp},  // vertex v2
			{-tmp, -tmp, -tmp},  // vertex v3
			{-tmp, tmp,  tmp},   // vertex v4
			{tmp,  tmp,  tmp},   // vertex v5
			{tmp,  tmp,  -tmp},  // vertex v6 
			{-tmp, tmp,  -tmp}   // vertex v7
	}; 
	
	static const int triangle[12][3] = {
			{0, 1, 4},  // polygon v0,v1,v4
			{1, 5, 4},  // polygon v1,v5,v4
			{1, 2, 5},  // polygon v1,v2,v5
			{2, 6, 5},  // polygon v2,v6,v5
			{2, 3, 6},  // polygon v2,v3,v6
			{3, 7, 6},  // polygon v3,v7,v6
			{3, 0, 7},  // polygon v3,v0,v7
			{0, 4, 7},  // polygon v0,v4,v7
			{4, 5, 7},  // polygon v4,v5,v7
			{5, 6, 7},  // polygon v5,v6,v7
			{3, 2, 0},  // polygon v3,v2,v0
			{2, 1, 0}   // polygon v2,v1,v0
	};
	
	static const float texCoord[8][2] = {
			{0.0f, 0.0f},  // mapping coordinates for vertex v0
			{1.0f, 0.0f},  // mapping coordinates for vertex v1
			{1.0f, 0.0f},  // mapping coordinates for vertex v2
			{0.0f, 0.0f},  // mapping coordinates for vertex v3
			{0.0f, 1.0f},  // mapping coordinates for vertex v4
			{1.0f, 1.0f},  // mapping coordinates for vertex v5
			{1.0f, 1.0f},  // mapping coordinates for vertex v6 
			{0.0f, 1.0f}   // mapping coordinates for vertex v7
	};

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 12; i++)
    {
        Vector v1 = Vector(vertex[triangle[i][1]]) - Vector(vertex[triangle[i][0]]);
        Vector v2 = Vector(vertex[triangle[i][2]]) - Vector(vertex[triangle[i][0]]);
        Vector n = v1 ^ v2;
        n.norm();
        glNormal3fv(n.v);

        glTexCoord2f(texCoord[triangle[i][0]][0], texCoord[triangle[i][0]][1]);
        glVertex3fv(vertex[triangle[i][0]]);

        glTexCoord2f(texCoord[triangle[i][1]][0], texCoord[triangle[i][1]][1]);
        glVertex3fv(vertex[triangle[i][1]]);

        glTexCoord2f(texCoord[triangle[i][2]][0], texCoord[triangle[i][2]][1]);
        glVertex3fv(vertex[triangle[i][2]]);
    }
	glEnd();

/*
#   define V(a,b,c) glVertex3f(a tmp, b tmp, c tmp);
#   define N(a,b,c) glNormal3f(a, b, c);


    glBegin(GL_QUADS);
        N( 1.0, 0.0, 0.0); 
            V(+,-,+); glTexCoord2f(1, 0);
            V(+,-,-); glTexCoord2f(0, 0);
            V(+,+,-); glTexCoord2f(0, 1);
            V(+,+,+); glTexCoord2f(1, 1);
        
        N( 0.0, 1.0, 0.0); 
            V(+,+,+); glTexCoord2f(1, 0);
            V(+,+,-); glTexCoord2f(0, 0);
            V(-,+,-); glTexCoord2f(0, 1);
            V(-,+,+); glTexCoord2f(1, 1);

        N( 0.0, 0.0, 1.0);
            V(+,+,+); glTexCoord2f(1, 0);
            V(-,+,+); glTexCoord2f(0, 0);
            V(-,-,+); glTexCoord2f(0, 1);
            V(+,-,+); glTexCoord2f(1, 1);

        N(-1.0, 0.0, 0.0);
            V(-,-,+); glTexCoord2f(1, 0);
            V(-,+,+); glTexCoord2f(0, 0);
            V(-,+,-); glTexCoord2f(0, 1);
            V(-,-,-); glTexCoord2f(1, 1);

        N( 0.0,-1.0, 0.0);
            V(-,-,+); glTexCoord2f(1, 0);
            V(-,-,-); glTexCoord2f(0, 0);
            V(+,-,-); glTexCoord2f(0, 1);
            V(+,-,+); glTexCoord2f(1, 1);

        N( 0.0, 0.0,-1.0);
            V(-,-,-); glTexCoord2f(1, 0);
            V(-,+,-); glTexCoord2f(0, 0);
            V(+,+,-); glTexCoord2f(0, 1);
            V(+,-,-); glTexCoord2f(1, 1);
    glEnd();

#   undef V
#   undef N
*/
}

void Video::renderFace(const Face& face, vector<vector<int>>& uv) const
{
    glBegin(GL_QUADS);
    for (size_t i = 0; i < face.size(); i++)
    {
        glTexCoord2i(uv[i][0], uv[i][1]);
        
        glVertex3fv(face[i].v);
    }
	glEnd();
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

void Video::beginObject(const Matrix& matrix) const
{
    glPushMatrix();
    glMultMatrixf(matrix.m);
}

void Video::endObject() const
{
    glPopMatrix();
}

unsigned int Video::loadTexture(const string& name) const
{
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
    
    return texture;
}
