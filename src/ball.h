#ifndef __BALL_H__
#define __BALL_H__

#include "common.h"
#include "body.h"

class Sound;
class SoundBuffer;
class Referee;
class Collision;


struct TriggerFlags
{
    TriggerFlags();
    void loadDefaults();

    bool m_wasTriggeredBefore;
    bool m_shouldRegisterCollision;
    bool m_hasTriggered;
};

typedef map<const Body*, TriggerFlags> TriggerFilterMap;

static const float BALL_RADIUS = 0.2f;

class Ball : public Collideable
{
public:
    Ball(Body* body, const Collision* levelCollision);
    virtual ~Ball() {}

    Vector getPosition() const;
    Vector getVelocity() const;
    void   setPosition0();

    // maybe private
    void onCollide(const Body* other, const NewtonMaterial* material);
    void onCollideHull(const Body* other, const NewtonMaterial* material);
    void onImpact(const Body* other, const Vector& position, float speed);
    void onScratch(const Body* other, const Vector& position, float speed);
    void triggerBegin();
    void triggerEnd();
    void addBodyToFilter(const Body* body);

    void renderShadow(const Vector& lightPosition) const;

    Referee*            m_referee;
    Body*               m_body;
    Sound*              m_sound;
    SoundBuffer*        m_soundBallGround;

private:
    TriggerFilterMap m_filteredBodies;
    
    const Collision* m_levelCollision;
};

#endif
