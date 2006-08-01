#include <Newton.h>

#include "materials.h"
#include "material.h"
#include "game.h"
#include "world.h"

Materials::Materials(const Game* game) : m_game(game)
{
}

Materials::~Materials()
{
    NewtonMaterialDestroyAllGroupID(m_game->m_world->m_newtonWorld);
    for each_(MaterialMap, m_materials, iter)
    {
        delete iter->second.material;
    }
}

void Materials::add(const string& name, Material* material)
{
    MaterialMap::iterator iter = m_materials.find(name);
    if (iter != m_materials.end())
    {
        throw Exception("Material '" + name + "' already exists");
    }
    int id = NewtonMaterialCreateGroupID(m_game->m_world->m_newtonWorld);
    m_materials.insert(make_pair(name, MaterialID(id, material)));
}

Material* Materials::get(const string& name)
{
    MaterialMap::iterator iter = m_materials.find(name);
    if (iter == m_materials.end())
    {
        throw Exception("Material '" + name + "' not found");
    }
    return iter->second.material;
}
