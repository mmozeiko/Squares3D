#ifndef __BALL_H__
#define __BALL_H__

#include "body.h"

class Referee;

struct TriggerFlags
{
    TriggerFlags();
    void loadDefaults();

    bool m_wasTriggeredBefore;
    bool m_shouldRegisterCollision;
};

typedef map<const Body*, TriggerFlags> TriggerFilterMap;

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
    void addBodyToFilter(const Body* body);

    Referee*            m_referee;
    Body*               m_body;

private:
    TriggerFilterMap m_filteredBodies;
    bool m_hasTriggered;
};

#endif
