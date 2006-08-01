#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <Newton.h>
#include "common.h"
#include "vmath.h"

class XMLnode;
class Game;
class Video;

class Material;    
class Materials;
class Collision;
class Body;

typedef map<string, Collision*> CollisionsMap;
typedef map<string, Body*>      BodiesMap;

string getAttribute(const XMLnode& node, const string& name);
string getAttribute(const XMLnode& node, const string& name, const string& defaultValue);
Vector getAttributesInVector(const XMLnode& node, const string& attributeSymbols);

class Level
{
public:
    Level(const Game* game);
    ~Level();
    void  load(const string& levelFile, StringSet& loaded = StringSet());
    void  render(const Video* video) const;
    void  prepare();
    Body* getBody(const string id);

    BodiesMap     m_bodies;
    CollisionsMap m_collisions;
    Materials*    m_materials;

private:
    const Game* m_game;
};


#endif
