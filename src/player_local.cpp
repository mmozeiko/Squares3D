#include "player_local.h"
#include "input.h"
#include "camera.h"
#include "world.h"
#include "config.h"

LocalPlayer::LocalPlayer(const Profile* profile, const Character* character, Level* level) :
    Player(profile, character, level) //Config::instance->m_misc.mouse_sensitivity)
{
    int w, h; 
    glfwGetWindowSize(&w, &h);
    m_lastMouse = make_pair(w/2, h/2);

}

LocalPlayer::~LocalPlayer()
{
}

void LocalPlayer::control()
{
    const Mouse& mouse = Input::instance->mouse();
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
    
    /*float t;
    if (dx > 0)
    {
        t = 1.0f;
    }
    else if (dx < 0)
    {
        t = -1.0f;
    }
    else
    {
        t = 0.0f;
    }
    setRotation(Vector(0.0f, t, 0.0f));
    */

    Vector direction(static_cast<float>(dx), 0, static_cast<float>(-dy));
    direction.norm();

    /*if (Input::instance->key('D')) direction.x =  1.0f;
    if (Input::instance->key('A')) direction.x = -1.0f;
    if (Input::instance->key('W')) direction.z =  1.0f;
    if (Input::instance->key('S')) direction.z = -1.0f;
    */

    setDirection(Matrix::rotateY(World::instance->m_camera->angleY()) * direction);
    setJump(Input::instance->key(GLFW_KEY_SPACE));

    Body* ball = World::instance->m_level->getBody("football");

    Vector ballPosition = ball->getPosition();
    Vector selfPosition = m_body->getPosition();

    Vector dir = ballPosition - selfPosition;
    Vector rot = m_body->getRotation();
    Vector rotation;
    rotation.y = ( rot % dir );

    setRotation(rotation);
}
