#include "ball.h"
#include "video.h"
#include "game.h"

Ball::Ball(const NewtonWorld* world, const Vector& pos, Game* game, const float radius) :
    Body(world, BallBody), m_radius(radius)
{
    NewtonCollision* collision = NewtonCreateSphere(m_world, m_radius, m_radius, m_radius, NULL);
    Body::create(collision, Matrix::translate(pos));

    Vector intertia, origin;
    NewtonConvexCollisionCalculateInertialMatrix(collision, intertia.v, origin.v);
    NewtonBodySetMassMatrix(m_body, 1.0f, intertia.x, intertia.y, intertia.z);

    NewtonBodySetContinuousCollisionMode(m_body, 1);
    
    m_texture = game->video()->loadTexture("ball.tga");
}

Ball::~Ball()
{
    glDeleteTextures(1, &m_texture);
}

void Ball::onSetForceAndTorque()
{
    NewtonBodyAddForce(m_body, gravityVec.v);
}

void Ball::onRender(const Video* video) const
{
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    video->renderSphere(m_radius);
    glDisable(GL_TEXTURE_2D);
}
