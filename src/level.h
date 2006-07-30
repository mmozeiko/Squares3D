#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "common.h"
#include "vmath.h"
#include "newton.h"

class XMLnode;
class Game;
class Video;

class Material;    
class Collision;
class Body;

typedef map<string, Collision*> CollisionsMap;
typedef map<string, Body*>      BodiesMap;
typedef map<string, Material*>  MaterialsMap;

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
    MaterialsMap  m_materials;

private:
    const Game* m_game;
};


#endif
