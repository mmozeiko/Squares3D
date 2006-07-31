#ifndef __COLLISION_H__
#define __COLLISION_H__

#include <Newton.h>
#include "common.h"
#include "vmath.h"

class Body;
class XMLnode;
class Video;
class Material;
typedef map<string, Material*>  MaterialsMap;

class Collision
{
    friend class Body;

public:
    static Collision* create(const XMLnode& node, const NewtonWorld* newtonWorld, const MaterialsMap* materials);
    
    virtual void render(const Video* video) const = 0;

    NewtonCollision*  m_newtonCollision;

    virtual ~Collision();

protected:
    Collision(const XMLnode& node, const NewtonWorld* newtonWorld);

    void create(NewtonCollision* collision);
    void create(NewtonCollision* collision, float mass);

private:
    const NewtonWorld*     m_newtonWorld;
    Vector                 m_origin;
    Vector                 m_inertia;
    float                  m_mass;
};


#endif