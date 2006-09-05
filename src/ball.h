#ifndef __BALL_H__
#define __BALL_H__

#include "body.h"

class Referee;

class Ball : public Collideable
{
public:
    Ball(Body* body);

    Vector getPosition();
    void   setPosition0();

    // maybe private
    void onCollide(Body* other, const NewtonMaterial* material);
    void onCollideHull(Body* other, const NewtonMaterial* material);
    void triggerBegin();
    void triggerEnd();
    void addBodyToFilter(const Body* body)
    {
        m_filteredBodies.insert(body);
    }

    Referee*            m_referee;
    Body*               m_body;

private:
    set<const Body*> m_filteredBodies;
    bool m_wasTriggeredBefore;
    bool m_hasCollidedWithBall;
    bool m_shouldRegisterCollision;

};

#endif
