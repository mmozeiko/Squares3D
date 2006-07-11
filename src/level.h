#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "common.h"
#include "vmath.h"

class XMLnode;

class LevelCollision
{
    friend class LevelBody;
private:
    LevelCollision(const XMLnode& node);
public:
    string m_type;      // ""
    float  m_mass;      // 1.0f
    Vector m_size;      // (1.0f, 1.0f, 1.0f)
    Vector m_position;  // (0.0f, 0.0f, 0.0f)
    Vector m_rotation;  // (0.0f, 0.0f, 0.0f)
    string m_texPath;   // ""
    Vector m_cAmbient;  // (0.2f, 0.2f, 0.2f, 1.0f)
    Vector m_cDiffuse;  // (0.8f, 0.8f, 0.8f, 1.0f)
    Vector m_cSpecular; // (0.0f, 0.0f, 0.0f)
    Vector m_cEmission; // (0.0f, 0.0f, 0.0f, 1.0f)
    float  m_cShine;    // 0.0f
};

class LevelBody
{
    friend class Level;
private:
    LevelBody(const XMLnode& node);
    ~LevelBody();

public:
    string               m_id;          // ""
    string               m_material;    // ""
    Vector               m_position;    // (0.0f, 0.0f, 0.0f)
    Vector               m_rotation;    // (0.0f, 0.0f, 0.0f)
    set<LevelCollision*> m_collisions;

};

class Level
{
public:
    Level();
    ~Level();
    void loadLevelData(const string& levelFile);
    set<LevelBody*> m_bodies;
};

#endif