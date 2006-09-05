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

class Collideable
{
public:
    // TODO: maybe somehow remove NewtonMaterial, or replace with Property class?
    virtual void onCollide(const Body* other, const NewtonMaterial* material) {}
    virtual void onCollideHull(const Body* other, const NewtonMaterial* material) {}
    virtual void onImpact(const Body* other, const Vector& position, const float speed) {}
    virtual void onScratch(const Body* other, const Vector& position, const float speed) {}

    virtual void onSetForceAndTorque() {}
};

class Body : public Collideable, NoCopy
{
    friend class Level;

public:
    void prepare();
    void render() const;
    void setTransform(const Vector& position, const Vector& rotation);
    Vector getPosition() const;
    Vector getRotation();
    void setCollideable(Collideable* collideable);

    void onCollide(const Body* other, const NewtonMaterial* material);
    void onCollideHull(const Body* other, const NewtonMaterial* material);
    void onImpact(const Body* other, const Vector& position, const float speed);
    void onScratch(const Body* other, const Vector& position, const float speed);

    string              m_id;
    NewtonBody*         m_newtonBody;        
    Matrix              m_matrix;
    CollisionSet        m_collisions;

    // TODO: move to cpp file
    float getMass() const
    {
        return m_totalMass;
    }

    const Vector& getInertia() const
    {
        return m_totalInertia;
    }

protected:

    Body(const XMLnode& node);
    ~Body();

    void createNewtonBody(const NewtonCollision* newtonCollision,
                          const Vector& totalOrigin,
                          const Vector& position,
                          const Vector& rotation);

private:

    float           m_totalMass;
    Vector          m_totalInertia;
    Collideable*    m_collideable;

    void onSetForceAndTorque();
    static void onSetForceAndTorque(const NewtonBody* newtonBody);
};


#endif
