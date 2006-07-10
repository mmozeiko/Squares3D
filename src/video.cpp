#include "video.h"

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

Video::Video(Config& config) : _config(config)
{
    clog << "Initializing video." << endl;

    if (glfwInit() != GL_TRUE)
    {
        throw Exception("glfwInit failed");
    }

    int width = _config.video().width;;
    int height = _config.video().height;
    bool vsync = _config.video().vsync;
    int mode = (_config.video().fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW);
    bool systemKeys = config.misc().system_keys;

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

void Video::RenderCube(float size) const
{
    float tmp = 0.5f * size;

#   define V(a,b,c) glVertex3f(a tmp, b tmp, c tmp);
#   define N(a,b,c) glNormal3f(a, b, c);

    glBegin(GL_QUADS);
        N( 1.0, 0.0, 0.0); V(+,-,+); V(+,-,-); V(+,+,-); V(+,+,+);
        N( 0.0, 1.0, 0.0); V(+,+,+); V(+,+,-); V(-,+,-); V(-,+,+);
        N( 0.0, 0.0, 1.0); V(+,+,+); V(-,+,+); V(-,-,+); V(+,-,+);
        N(-1.0, 0.0, 0.0); V(-,-,+); V(-,+,+); V(-,+,-); V(-,-,-);
        N( 0.0,-1.0, 0.0); V(-,-,+); V(-,-,-); V(+,-,-); V(+,-,+);
        N( 0.0, 0.0,-1.0); V(-,-,-); V(-,+,-); V(+,+,-); V(+,-,-);
    glEnd();

#   undef V
#   undef N
}

void Video::RenderSphere(float radius) const
{
    GLUquadric* q =  gluNewQuadric();
    gluSphere(q, radius, 32, 32);
    gluDeleteQuadric(q);
}
    
void Video::RenderWireSphere(float radius) const
{
    GLUquadric* q =  gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_SILHOUETTE);
    gluSphere(q, radius, 32, 32);
    gluDeleteQuadric(q);
}
  
void Video::RenderAxes(float size) const
{
    
    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(-size, 0.0, 0.0);
    glVertex3f(size, 0.0, 0.0);

    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, -size, 0.0);
    glVertex3f(0.0, size, 0.0);

    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, -size);
    glVertex3f(0.0, 0.0, size);
    glEnd();

    glEnable(GL_LIGHTING);
    
    GLUquadric* q =  gluNewQuadric();
    
    glPushMatrix();
    const float red[] = {1.0, 0.0, 0.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
    glTranslatef(size, 0.0, 0.0);
    glRotatef(90.0, 0.0, 1.0, 0.0);
    gluCylinder(q, 0.2, 0.0, 1.0, 32, 32);
    glRotatef(180.0, 0.0, 1.0, 0.0);
    gluDisk(q, 0.0, 0.2, 32, 32);
    glPopMatrix();

    glPushMatrix();
    const float green[] = {0.0, 1.0, 0.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
    glTranslatef(0.0, size, 0.0);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    gluCylinder(q, 0.2, 0.0, 1.0, 32, 32);
    glRotatef(180.0, 0.0, 1.0, 0.0);
    gluDisk(q, 0.0, 0.2, 32, 32);
    glPopMatrix();

    glPushMatrix();
    const float blue[] = {0.0, 0.0, 1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
    glTranslatef(0.0, 0.0, size);
    gluCylinder(q, 0.2, 0.0, 1.0, 32, 32);
    glRotatef(180.0, 0.0, 1.0, 0.0);
    gluDisk(q, 0.0, 0.2, 32, 32);
    glPopMatrix();

    gluDeleteQuadric(q);
}

void Video::BeginObject(const Matrix& matrix) const
{
    glPushMatrix();
    glMultMatrixf(matrix.m);
}

void Video::EndObject() const
{
    glPopMatrix();
}

