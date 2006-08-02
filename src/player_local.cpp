#include "player_local.h"
#include "input.h"
#include "camera.h"
#include "game.h"
#include "world.h"

LocalPlayer::LocalPlayer(const string& id, const Game* game, const Vector& position, const Vector& rotation) :
    Player(id, game, position, rotation)
{
    int w, h; 
    glfwGetWindowSize(&w, &h);
    m_lastMouse = make_pair(w/2, h/2);

}

LocalPlayer::~LocalPlayer()
{
}

void LocalPlayer::control(const Input* input)
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
    
    float t;
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

    Vector direction;

    if (input->key('D')) direction.x =  1.0f;
    if (input->key('A')) direction.x = -1.0f;
    if (input->key('W')) direction.z =  1.0f;
    if (input->key('S')) direction.z = -1.0f;
    
    if (input->key(GLFW_KEY_SPACE)) 
    {
        direction.y += 1.0f;
    }
    
    setDirection(Matrix::rotateY(m_game->m_world->m_camera->angleY()) * direction );
}
