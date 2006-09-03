#include <Newton.h>

#include "ball.h"
#include "video.h"
#include "game.h"
#include "referee.h"
#include "collision.h"
#include "world.h"

Ball::Ball(Body* body, 
           const Game* game) :
    m_body(body),
    m_game(game)
{
    m_body->setCollideable(this);
    
    NewtonCollision* ballCollision = (*m_body->m_collisions.begin())->m_newtonCollision;
    NewtonCollision* hull = NewtonCreateConvexHullModifier(m_game->m_world->m_newtonWorld, ballCollision);

    // hull scale - 5%
    static const float t = 3.05f;
    Matrix matrix = Matrix::scale(Vector(t, t, t));

    NewtonConvexHullModifierSetMatrix(hull, matrix.m);
    NewtonConvexCollisionSetUserID(hull, CollisionType_Hull);

    NewtonCollision* both[] = { ballCollision, hull };
    NewtonCollision* newCollision = NewtonCreateCompoundCollision(m_game->m_world->m_newtonWorld, sizeOfArray(both), both);
   
    NewtonBodySetCollision(m_body->m_newtonBody, newCollision);

    // TODO: wtf?
    //NewtonReleaseCollision(m_game->m_world->m_newtonWorld, hull);
    //NewtonReleaseCollision(m_game->m_world->m_newtonWorld, newCollision);
}

Vector Ball::getPosition()
{
    return m_body->getPosition();
}

void Ball::setPosition0()
{
    return m_body->setTransform(Vector(0,2,0), Vector(0,0,0));
}

void Ball::onCollide(Body* other, const NewtonMaterial* material)
{
    m_referee->process(m_body, other);
}

void Ball::onCollideHull(Body* other, const NewtonMaterial* material)
{
    // collision with convex hull
    1;
}
