#include "camera.h"
#include "common.h"
#include "input.h"
#include "game.h"

#include <GL/glfw.h>

static const float LOOK_SPEED = 2.0f;
static const float MOVE_SPEED = 10.0f;

/*
Translate(Camera.Position);
Rotate(0, 0, Camera.Yaw);
Rotate(0, Camera.Pitch, 0);
Rotate(Camera.Roll, 0, 0);



1. The forward view vector depends on yaw and pitch and can be calculated using a spherical coordinate conversion
2. The forward movement vector depends on yaw only and is calculated as usual using sin() and cos()
3. The side movement and view vectors are the same and are perpendicular to the forward movement vector in the ground plane
4. The view up vector is the cross product of the view forward and side vectors
5. The movement up vector (e.g. for jumping or spectator mode) is the world up vector


First of all, since people use different axes for the up axis (usually y or z), the code is set up generically to handle any up axis. So we start with this:

size_t i = m_upAxis;
size_t j = (i + 1) % 3;
size_t k = (j + 1) % 3;

Where m_upAxis is 0, 1 or 2 (your choice). We'll of course need the sine and cosine of our angles (remember to convert from degrees if needed):

float sy = sin(m_yaw);
float cy = cos(m_yaw);
float sp = sin(m_pitch);
float cp = cos(m_pitch);

The forward view and movement vectors are constructed like this:

m_forwardView[i] = sp;
m_forwardView[j] = cp * cy;
m_forwardView[k] = cp * sy;

m_forwardMove[i] = 0.0f;
m_forwardMove[j] = cy;
m_forwardMove[k] = sy;

As mentioned before, the side view and movement vectors are the same, and are perpendicular to the forward movement vector in the ground plane:

m_sideView[i] = 0.0f;
m_sideView[j] = -sy;
m_sideView[k] = cy;

m_sideMove = m_sideView;

The up view and movement vectors are constructed as described previously:

m_upView = m_forwardView.Cross(m_sideView);

m_upMove[i] = 1.0f;
m_upMove[j] = 0.0f;
m_upMove[k] = 0.0f;

And that's it. You can now use the three move vectors to respond to movement commands. The view vectors can be used for, say, firing projectiles, and can also be loaded into a matrix along with the position and submitted to the API of your choice. Just to make it really easy, to set up the camera in OpenGL, you can do this:

gluLookAt(
    m_pos[0],
    m_pos[1],
    m_pos[2],
    m_pos[0] + m_forwardView[0],
    m_pos[1] + m_forwardView[1],
    m_pos[2] + m_forwardView[2],
    m_upView[0],
    m_upView[1],
    m_upView[2]
);
*/


Camera::Camera(Game* game, const Vector& pos, float angleX, float angleY) :
    Renderable(game),
    m_pos(-pos), m_angleX(angleX), m_angleY(angleY),
    m_strafeRotation(Matrix::rotateY(-M_PI/2)),
    m_scaleMatrix(Matrix::scale(Vector(1.0f, 1.0f, -1.0f)))
{
    int w, h; 
    glfwGetWindowSize(&w, &h);
    m_lastMouse = make_pair(w/2, h/2);
}

Camera::~Camera()
{
}

void Camera::control(const Input* input)
{
    const Mouse& mouse = input->mouse();
    int dx = mouse.x - m_lastMouse.first;
    int dy = mouse.y - m_lastMouse.second;

    m_lastMouse = make_pair(mouse.x, mouse.y);

    int w, h;
    glfwGetWindowSize(&w, &h);
    int w2 = w/2, h2 = h/2;
    if (w2==0 || h2==0)
    {
        return;
    }

    if (glfwGetWindowParam(GLFW_ACTIVE) == GL_FALSE)
    {
        return;
    }

    //m_targetRotation = Vector(static_cast<float>(dy)/h2, static_cast<float>(dx)/w2, 0.0f);
    
    m_targetDirection = Vector();
    m_targetRotation = Vector();

    if (input->key(GLFW_KEY_UP)) m_targetDirection.z = +1.0f;
    if (input->key(GLFW_KEY_DOWN))  m_targetDirection.z = -1.0f;
    if (input->key(GLFW_KEY_RIGHT))  m_targetDirection.x = +1.0f;
    if (input->key(GLFW_KEY_LEFT)) m_targetDirection.x = -1.0f;

    if (input->key(GLFW_KEY_HOME)) m_targetRotation.x = -1.0f;
    if (input->key(GLFW_KEY_END))  m_targetRotation.x = +1.0f;
    if (input->key(GLFW_KEY_DEL))  m_targetRotation.y = -1.0f;
    if (input->key(GLFW_KEY_PAGEDOWN)) m_targetRotation.y = +1.0f;

    m_targetDirection.norm();
}

void Camera::update(float delta)
{
    m_targetRotation *= delta;
    m_targetDirection *= delta;

    m_angleY += LOOK_SPEED * m_targetRotation.y;
    m_angleX += LOOK_SPEED * m_targetRotation.x;

    Matrix moveMatrix = Matrix::rotateY(-m_angleY) * Matrix::rotateX(-m_angleX);
    Matrix strafeMatrix = moveMatrix * m_strafeRotation;

    Vector deltaPos = m_targetDirection.z * moveMatrix.row(2) + m_targetDirection.x * strafeMatrix.row(2);
    m_pos += MOVE_SPEED * deltaPos;
}

void Camera::prepare()
{
    m_matrix = Matrix::rotateX(m_angleX) * Matrix::rotateY(m_angleY) * 
               Matrix::translate(m_pos) * m_scaleMatrix;
}

void Camera::render(const Video* video) const
{
    glLoadMatrixf(m_matrix.m);
}

float Camera::angleY() const
{
    return m_angleY;
}
