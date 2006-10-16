#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <Newton.h>
#include "common.h"
#include "vmath.h"

class XMLnode;
class Material;    
class Properties;
class Collision;
class Body;
class Profile;
struct Face;

typedef map<string, Material*>       MaterialsMap;
typedef map<string, Collision*>      CollisionsMap;
typedef map<string, Body*>           BodiesMap;
typedef set<pair<Face*, Material*> > FaceSet;

class Level : NoCopy
{
public:
    Level();
    ~Level();
    void  load(const string& levelFile, StringSet& loaded = StringSet());
    void  render() const;
    void  prepare();
    Body* getBody(const string& id) const;
    Collision* getCollision(const string& id) const;

    BodiesMap       m_bodies;
    CollisionsMap   m_collisions;
    FaceSet         m_faces;
    MaterialsMap    m_materials;
    Properties*     m_properties;
};


#endif
