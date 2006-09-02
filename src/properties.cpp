#include <Newton.h>

#include "properties.h"
#include "property.h"
#include "game.h"
#include "world.h"
#include "level.h"
#include "body.h"

struct MaterialContact : NoCopy
{
    static int onBegin(const NewtonMaterial* material, const NewtonBody* body0, const NewtonBody* body1);
    static int onProcess(const NewtonMaterial* material, const NewtonContact* contact);
    static void onEnd(const NewtonMaterial* material);

    Body* body[2];
    
    Vector position;
    float  maxNormalSpeed;
    float  maxTangentSpeed;
};

Properties::Properties(const Game* game) : m_game(game)
{
}

Properties::~Properties()
{
    for each_(MaterialContactSet, m_materialContacts, iter)
    {
        delete *iter;
    }
    NewtonMaterialDestroyAllGroupID(m_game->m_world->m_newtonWorld);
}

int Properties::getPropertyID(const string& name)
{
    int result;
    IntMap::const_iterator iter = m_newtonMaterials.find(name);
    if (iter == m_newtonMaterials.end())
    {
        result = NewtonMaterialCreateGroupID(m_game->m_world->m_newtonWorld);
        m_newtonMaterials.insert(make_pair(name, result));
    }
    else
    {
        result = iter->second;
    }
    return result;
}

void Properties::load(const XMLnode& node)
{
    int id0 = getPropertyID(getAttribute(node, "property0"));
    int id1 = getPropertyID(getAttribute(node, "property1"));
    if (foundInMap(m_properties, makepID(id0, id1)))
    {
        throw Exception("Properties for '" + getAttribute(node, "property0") + "' and '"
                                           + getAttribute(node, "property1") + "' already loaded");
    }

    float sF = cast<float>(getAttribute(node, "staticFriction"));
    float kF = cast<float>(getAttribute(node, "kineticFriction"));
    float eC = cast<float>(getAttribute(node, "elasticityCoeficient"));
    float sC = cast<float>(getAttribute(node, "softnessCoeficient"));

    NewtonWorld* world = m_game->m_world->m_newtonWorld;
    NewtonMaterialSetDefaultElasticity(world, id0, id1, eC);
    NewtonMaterialSetDefaultFriction(world, id0, id1, sF, kF);
    NewtonMaterialSetDefaultSoftness(world, id0, id1, sC);
    
    MaterialContact* materialContact = new MaterialContact();
    
    NewtonMaterialSetCollisionCallback(
        world, id0, id1,
        static_cast<void*>(materialContact), 
        MaterialContact::onBegin, 
        MaterialContact::onProcess,
        MaterialContact::onEnd);

    m_properties.insert(make_pair(makepID(id0, id1), Property(sF, kF, eC, sC)));
    m_materialContacts.insert(materialContact);
}

pID Properties::makepID(int id0, int id1) const
{
    if (id0 > id1)
    {
        swap(id0, id1);
    }
    // now id0 always <= id1
    return (static_cast<pID>(id0) << 32) + id1;
}

int MaterialContact::onBegin(const NewtonMaterial* material, const NewtonBody* body0, const NewtonBody* body1)
{
    MaterialContact* self = static_cast<MaterialContact*>(NewtonMaterialGetMaterialPairUserData(material));
    self->body[0] = static_cast<Body*>(NewtonBodyGetUserData(body0));
    self->body[1] = static_cast<Body*>(NewtonBodyGetUserData(body1));

    self->maxNormalSpeed = 0.0f;
    self->maxTangentSpeed = 0.0f;

    return 1;
}

int MaterialContact::onProcess(const NewtonMaterial* material, const NewtonContact* contact)
{
    MaterialContact* self = static_cast<MaterialContact*>(NewtonMaterialGetMaterialPairUserData(material));
    
    Vector normal;

    float sp = NewtonMaterialGetContactNormalSpeed(material, contact);
    if (sp > self->maxNormalSpeed)
    {
        self->maxNormalSpeed = sp;
        NewtonMaterialGetContactPositionAndNormal(material, self->position.v, normal.v);
    }

    for (int i=0; i<2; i++)
    {
        float speed = NewtonMaterialGetContactTangentSpeed(material, contact, i);
        if (speed > self->maxTangentSpeed)
        {
            self->maxTangentSpeed = speed;
            NewtonMaterialGetContactPositionAndNormal(material, self->position.v, normal.v);
        }
    }
    
    // TODO: apply coefficients for tree collision, take in account material group of each face
    /*
    Material* mat = NULL;
    unsigned int c0 = NewtonMaterialGetBodyCollisionID(material, self->body[0]);
    unsigned int c1 = NewtonMaterialGetBodyCollisionID(material, self->body[1]);
    unsigned int faceAttr = NewtonMaterialGetContactFaceAttribute(material);
    if (collID != 0)
    {
        mat = *reinterpret_cast<Material**>(&collID);
    }
    else if (faceAttr != 0)
    {
            mat =*reinterpret_cast<Material**>(&collID);
    }

    if (mat != NULL)
    {
        // apply material
        // mat
    }
    */

    return 1;
}

void MaterialContact::onEnd(const NewtonMaterial* material)
{
    MaterialContact* self = static_cast<MaterialContact*>(NewtonMaterialGetMaterialPairUserData(material));
    if (self->body[0] == self->body[1])
    {
        assert(false);
        return;
    }

    self->body[0]->onCollide(self->body[1], material);
    self->body[1]->onCollide(self->body[0], material);

    if (self->maxNormalSpeed > 0.0f && self->maxNormalSpeed > self->maxTangentSpeed)
    {
        self->body[0]->onImpact(self->body[1], self->position, self->maxNormalSpeed);
        self->body[1]->onImpact(self->body[0], self->position, self->maxNormalSpeed);
    }
    else if (self->maxTangentSpeed > 0.0f && self->maxTangentSpeed > self->maxNormalSpeed)
    {
        self->body[0]->onScratch(self->body[1], self->position, self->maxTangentSpeed);
        self->body[1]->onScratch(self->body[0], self->position, self->maxTangentSpeed);
    }
}
