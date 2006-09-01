#include "ball.h"
#include "video.h"
#include "game.h"
#include "referee.h"

Ball::Ball(Body* body, 
           const Game* game) :
    m_body(body),
    m_game(game)
{
    m_body->setCollideable(this);
}

Vector Ball::getPosition()
{
    return m_body->getPosition();
}

void Ball::onCollide(Body* other, const NewtonMaterial* material)
{
	m_referee->process(m_body, other);
}