#include "player_local.h"

#include "input.h"
#include "camera.h"
#include "game.h"
#include "world.h"

LocalPlayer::LocalPlayer(Game* game, int material, const Vector& pos, const Vector& size) :
    Player(game, material, pos, size)
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
    
    Vector rotation(0.0f, static_cast<float>(dx)/w2, 0.0f);
    setRotation(rotation);

    Vector direction;

    if (input->key('D')) direction.x =  1.0f;
    if (input->key('A')) direction.x = -1.0f;
    if (input->key('W')) direction.z =  1.0f;
    if (input->key('S')) direction.z = -1.0f;
    
    if (input->key(GLFW_KEY_SPACE)) 
    {
        direction.y += 1.0f;
    }
    
    m_crouch = (input->key('C') ? 0.2f : 2.0f) / 2.0f;
    if (input->key('X')) m_crouch = 10.0f;

    setDirection(Matrix::rotateY(m_game->m_world->m_camera->angleY()) * direction );
}
