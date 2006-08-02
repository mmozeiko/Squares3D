#ifndef __MATERIALS_H__
#define __MATERIALS_H__

#include "common.h"

class Game;
class Material;
class XMLnode;

typedef map<string, Material*> MaterialMap;

struct Properties
{
    Properties(float sF, float kF, float eC, float sC) : 
        staticFriction(sF),
        kineticFriction(kF),
        elasticityCoefficient(eC),
        softnessCoefficient(sC) {}

    float staticFriction;
    float kineticFriction;
    float elasticityCoefficient;
    float softnessCoefficient;
};

typedef map<long long, Properties> PropertiesMap;

class Materials
{
public:
    Materials(const Game* game);
    ~Materials();

    void loadProperties(const XMLnode& node);

    void add(const string& name, Material* material);
    Material* get(const string& name) const;

private:
    MaterialMap   m_materials;
    PropertiesMap m_properties;
    const Game* m_game;

    long long makePID(int id1, int id2) const;
};

#endif
