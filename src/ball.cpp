#include "ball.h"
#include "video.h"
#include "game.h"
#include "shader.h"

#include <GL/glext.h>

Ball::Ball(Game* game, const Vector& pos, const float radius) :
    Body(game, BallBody), m_radius(radius)
{
    NewtonCollision* collision = NewtonCreateSphere(m_world, m_radius, m_radius, m_radius, NULL);
    Body::create(collision, Matrix::translate(pos));

    Vector intertia, origin;
    NewtonConvexCollisionCalculateInertialMatrix(collision, intertia.v, origin.v);
    NewtonBodySetMassMatrix(m_body, 1.0f, intertia.x, intertia.y, intertia.z);

    NewtonBodySetContinuousCollisionMode(m_body, 1);
    
    m_texture = m_game->m_video->loadTexture("ball.tga");
    m_textureBump = m_game->m_video->loadTexture("ball_bump.tga");
    m_shader = m_game->m_video->loadShader("bump.vp", "bump.fp");
}

Ball::~Ball()
{
}

void Ball::onSetForceAndTorque()
{
    NewtonBodyAddForce(m_body, gravityVec.v);
}

void Ball::render(const Video* video) const
{
    Video::glActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureBump);

    Video::glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    video->begin(m_matrix.m);

    glColor3f(1.0, 1.0, 1.0);
    
    video->begin(m_shader);
    video->renderSphere(m_radius);
    video->end(m_shader);
   
    video->end();

    glDisable(GL_TEXTURE_2D);
}

void Ball::control(const Input*)
{
}
