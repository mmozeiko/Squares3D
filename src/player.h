#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "common.h"
#include "body.h"
#include "profile.h"
#include "level.h"

class Referee;
class Collision;

static const float FIELDLENGTH = 3.0f;

class Player : public Collideable
{
public:
    Player(const Profile* profile, Level* level);
    virtual ~Player();

    void setDirection(const Vector& direction);
    void setRotation(const Vector& rotation);
    void setJump(bool needJump);
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

    Vector       m_direction;
    Vector       m_rotation;

    const Collision* m_levelCollision;
};

#endif
