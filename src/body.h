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

class Collideable
{
public:
    virtual void onImpact(const Vector& position, const float speed) {}
    virtual void onScratch(const Vector& position, const float speed) {}
};

class Body
{
    friend class Level;

public:
    void prepare();
    void render(const Video* video);
    void setTransform(const Vector& position, const Vector& rotation);
    Vector getPosition();
    Vector getRotation();
    bool isCollideable() const;
    void setCollideable(Collideable* collideable);

    NewtonBody*     m_newtonBody;        

protected:

    Body(const XMLnode& node, const Game* game);

    const NewtonWorld*     m_newtonWorld;
    Matrix                 m_matrix;

    ~Body();

    void createNewtonBody(const NewtonCollision* newtonCollision,
                          const Vector& totalOrigin,
                          const Vector& position,
                          const Vector& rotation);

private:

    float           m_totalMass;
    Vector          m_totalInertia;
    set<Collision*> m_collisions;
    Collideable*    m_collideable;

    void onSetForceAndTorque();
    static void onSetForceAndTorque(const NewtonBody* newtonBody);
};


#endif
