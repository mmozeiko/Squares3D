#ifndef __BODY_H__
#define __BODY_H__

#include <Newton.h>
#include "common.h"
#include "vmath.h"

class Collision;
class Level;
class XMLnode;
class Body;

typedef set<const Collision*> CollisionSet;

class Collideable : NoCopy
{
public:
    virtual ~Collideable() {}

    // TODO: maybe somehow remove NewtonMaterial, or replace with Property class?
    virtual void onCollide(const Body* other, const NewtonMaterial* material, const Vector& position, float speed) {}
    virtual void onCollideHull(const Body* other) {}

    virtual void onSetForceAndTorque() {}
};

class Body : public Collideable
{
    friend class Level;

public:
    void prepare();
    void render() const;
    
    void setTransform(const Vector& position, const Vector& rotation);
    void setKickForce(const Vector& force);

    Vector getPosition() const;
    Vector getRotation() const;
    Vector getVelocity() const;

    void setCollideable(Collideable* collideable);

    Body(const string& id, Level* level, const Collision* collision);
    virtual ~Body();

    void onCollide(const Body* other, const NewtonMaterial* material, const Vector& position, float speed);
    void onCollideHull(const Body* other);

    string              m_id;
    NewtonBody*         m_newtonBody;        
    Matrix              m_matrix;
    CollisionSet        m_collisions;

    float getMass() const;
    const Vector& getInertia() const;

    bool m_soundable;
    bool m_important;

protected:

    Body(const XMLnode& node, Level* level);

    void createNewtonBody(const NewtonCollision* newtonCollision,
                          const Vector& totalOrigin,
                          const Vector& position,
                          const Vector& rotation);

private:

    float        m_totalMass;
    Vector       m_totalInertia;
    Collideable* m_collideable;
    Vector       m_velocity;
    Vector       m_kickForce;
    const Level* m_level;

    void onSetForceAndTorque();
    static void onSetForceAndTorque(const NewtonBody* newtonBody);
};


#endif
