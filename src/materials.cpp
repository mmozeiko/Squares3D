#include <Newton.h>

#include "materials.h"
#include "material.h"
#include "game.h"
#include "world.h"
#include "level.h"

struct MaterialContact
{
    static int onBegin(const NewtonMaterial* material, const NewtonBody* body1, const NewtonBody* body2);
    static int onProcess(const NewtonMaterial* material, const NewtonContact* contact);
    static void onEnd(const NewtonMaterial* material);

    Body* body1;
    Body* body2;
	
    Vector position;
	float  maxNormalSpeed;
	float  maxTangentSpeed;
};

Materials::Materials(const Game* game) : m_game(game)
{
}

Materials::~Materials()
{
    for each_(MaterialMap, m_materials, iter)
    {
        delete iter->second;
    }
    for each_(MaterialContactSet, m_materialContacts, iter)
    {
        delete *iter;
    }
    NewtonMaterialDestroyAllGroupID(m_game->m_world->m_newtonWorld);
}

void Materials::add(const string& name, Material* material)
{
    MaterialMap::const_iterator iter = m_materials.find(name);
    if (iter != m_materials.end())
    {
        throw Exception("Material '" + name + "' already exists");
    }
    m_materials.insert(make_pair(name, material));
}

Material* Materials::get(const string& name) const
{
    MaterialMap::const_iterator iter = m_materials.find(name);
    if (iter == m_materials.end())
    {
        throw Exception("Material '" + name + "' not found");
    }
    return iter->second;
}

void Materials::loadProperties(const XMLnode& node)
{
    const Material* m1 = get(getAttribute(node, "material1"));
    const Material* m2 = get(getAttribute(node, "material2"));
    float sF = cast<float>(getAttribute(node, "staticFriction"));
    float kF = cast<float>(getAttribute(node, "kineticFriction"));
    float eC = cast<float>(getAttribute(node, "elasticityCoeficient"));
    float sC = cast<float>(getAttribute(node, "softnessCoeficient"));

    NewtonWorld* world = m_game->m_world->m_newtonWorld;
    int id1 = m1->m_newtonID;
    int id2 = m2->m_newtonID;
    NewtonMaterialSetDefaultElasticity(world, id1, id2, eC);
    NewtonMaterialSetDefaultFriction(world, id1, id2, sF, kF);
    NewtonMaterialSetDefaultSoftness(world, id1, id2, sC);
    
    MaterialContact* materialContact = new MaterialContact();
    
    NewtonMaterialSetCollisionCallback(
        world, id1, id2,
        static_cast<void*>(materialContact), 
        MaterialContact::onBegin, 
        MaterialContact::onProcess,
        MaterialContact::onEnd);

    m_properties.insert(make_pair(makePID(id1, id2), Properties(sF, kF, eC, sC)));
    m_materialContacts.insert(materialContact);
}

long long Materials::makePID(int id1, int id2) const
{
    if (id1 > id2)
    {
        swap(id1, id2);
    }
    // now id1 always <= id2
    return (static_cast<long long>(id1) << 32) + id2;
}

int MaterialContact::onBegin(const NewtonMaterial* material, const NewtonBody* body1, const NewtonBody* body2)
{
    MaterialContact* self = static_cast<MaterialContact*>(NewtonMaterialGetMaterialPairUserData(material));
    self->body1 = static_cast<Body*>(NewtonBodyGetUserData(body1));
    self->body2 = static_cast<Body*>(NewtonBodyGetUserData(body2));

	self->maxNormalSpeed = 0.0f;
	self->maxTangentSpeed = 0.0f;

    return 1;
}

int MaterialContact::onProcess(const NewtonMaterial* material, const NewtonContact* contact)
{
    MaterialContact* self = static_cast<MaterialContact*>(NewtonMaterialGetMaterialPairUserData(material));
    
    return 1;
}

void MaterialContact::onEnd(const NewtonMaterial* material)
{
    MaterialContact* self = static_cast<MaterialContact*>(NewtonMaterialGetMaterialPairUserData(material));
}
