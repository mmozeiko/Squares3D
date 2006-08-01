#ifndef __MATERIALS_H__
#define __MATERIALS_H__

#include "common.h"

class Game;
class Material;

struct MaterialID
{
    MaterialID(int id, Material* material) : id(id), material(material) {}
    int id;
    Material* material;
};
typedef map<string, MaterialID> MaterialMap;

class Materials
{
public:
    Materials(const Game* game);
    ~Materials();

    void add(const string& name, Material* material);
    Material* get(const string& name);

private:
    MaterialMap m_materials;
    const Game* m_game;
};

#endif
