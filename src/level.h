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
struct Face;

typedef map<string, Material*>       MaterialMap;
typedef map<string, Collision*>      CollisionsMap;
typedef map<string, Body*>           BodiesMap;
typedef set<pair<Face*, Material*> > FaceSet;

Vector getAttributesInVector(const XMLnode& node, const string& attributeSymbols);

class Level : NoCopy
{
public:
    Level();
    ~Level();
    void  load(const string& levelFile, StringSet& loaded = StringSet());
    void  render() const;
    void  prepare();
    Body* getBody(const string id);

    BodiesMap     m_bodies;
    CollisionsMap m_collisions;
    FaceSet       m_faces;
    MaterialMap   m_materials;
    Properties*   m_properties;
};


#endif
