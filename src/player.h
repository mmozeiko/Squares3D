#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "common.h"
#include "body.h"
#include "level.h"
#include "timer.h"

class Referee;
class Collision;

static const float FIELDLENGTH = 3.0f;

class Player : public Collideable
{
public:
    Player(const Profile* profile, Level* level);
    virtual ~Player();

    virtual void halt() {}
    
    void setDirection(const Vector& direction);
    void setRotation(const Vector& rotation);
    void setJump(bool needJump);
    void setKick(const Vector& kick);

    void setPositionRotation(const Vector& position, const Vector& rotation);

    virtual void control() = 0;

    Vector getPosition() const;
    Vector getFieldCenter() const;

    void renderColor() const;

    //todo: maybe private
    void onCollide(const Body* other, const NewtonMaterial* material);
    void onImpact(const Body* other, const Vector& position, float speed);
    void onScratch(const Body* other, const Vector& position, float speed);
    void onSetForceAndTorque();
    
    Referee*          m_referee;
    Body*             m_body;
    const Profile*    m_profile;
    //player must recognize his field
    Vector            m_lowerLeft;
    Vector            m_upperRight;

protected:
    NewtonJoint* m_upVector;

    bool         m_isOnGround; // TODO: rename, current name is incorrect
    bool         m_jump;
    bool         m_halt;

    Vector       m_direction;
    Vector       m_rotation;
    Timer        m_timer;

    const Collision* m_levelCollision;
    Body*            m_ballBody;
};

#endif
