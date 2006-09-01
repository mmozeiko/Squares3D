#ifndef __BODY_H__
#define __BODY_H__

#include <Newton.h>
#include "common.h"
#include "vmath.h"

class Collision;
class Level;
class Video;
class XMLnode;
class Game;
class Body;

class Collideable
{
public:
    // TODO: maybe somehow remove NewtonMaterial, or replace with Property class?
    virtual void onCollide(Body* other, const NewtonMaterial* material) {}
    virtual void onImpact(Body* other, const Vector& position, const float speed) {}
    virtual void onScratch(Body* other, const Vector& position, const float speed) {}

    virtual void onSetForceAndTorque() {}
};

class Body : public Collideable, NoCopy
{
    friend class Level;

public:
    void prepare();
    void render(const Video* video);
    void setTransform(const Vector& position, const Vector& rotation);
    Vector getPosition() const;
    Vector getRotation();
    void setCollideable(Collideable* collideable);

    void onCollide(Body* other, const NewtonMaterial* material);
    void onImpact(Body* other, const Vector& position, const float speed);
    void onScratch(Body* other, const Vector& position, const float speed);

    NewtonBody* m_newtonBody;        
    Matrix      m_matrix;

protected:

    Body(const XMLnode& node, const Game* game);
    ~Body();

    const NewtonWorld*     m_newtonWorld;

    void createNewtonBody(const NewtonCollision* newtonCollision,
                          const Vector& totalOrigin,
                          const Vector& position,
                          const Vector& rotation,
                          const int     materialID);

private:

    float           m_totalMass;
    Vector          m_totalInertia;
    set<Collision*> m_collisions;
    Collideable*    m_collideable;

    void onSetForceAndTorque();
    static void onSetForceAndTorque(const NewtonBody* newtonBody);
};


#endif
