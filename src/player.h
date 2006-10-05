#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "common.h"
#include "level.h"
#include "body.h"
#include "level.h"

class Referee;

#define FIELDLENGTH 3.0f

class Player : public Collideable
{
public:
    Player(const XMLnode& node, const Level* level);
    virtual ~Player();

    void setDirection(const Vector& direction);
    void setRotation(const Vector& rotation);
    void setDisplacement(const Vector& position, const Vector& rotation);

    virtual void control() = 0;

    Vector getPosition() const;
    Vector getFieldCenter() const;

    //todo: maybe private
    void onCollide(const Body* other, const NewtonMaterial* material);
    void onImpact(const Body* other, const Vector& position, const float speed);
    void onScratch(const Body* other, const Vector& position, const float speed);
    void onSetForceAndTorque();
    
    Referee*     m_referee;
    Body*        m_body;
    //player must recognize his field
    Vector       m_lowerLeft;
    Vector       m_upperRight;
    Vector       m_color;
    float        m_speed;
    float        m_accuracy;
    string       m_name;

protected:
    NewtonJoint* m_upVector;

    bool         m_isOnGround; // TODO: rename, current name is incorrect
    Vector       m_direction;
    Vector       m_rotation;
};

#endif
