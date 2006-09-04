#include <Newton.h>

#include "properties.h"
#include "property.h"
#include "world.h"
#include "level.h"
#include "body.h"
#include "collision.h"

struct MaterialContact : NoCopy
{
    static int onBegin(const NewtonMaterial* material, const NewtonBody* body0, const NewtonBody* body1);
    static int onProcess(const NewtonMaterial* material, const NewtonContact* contact);
    static void onEnd(const NewtonMaterial* material);

    Body* body[2];
    const Properties* properties;
	
    Vector position;
	float  maxNormalSpeed;
	float  maxTangentSpeed;
};

Properties::Properties() : m_uniqueID(2)
{
    NewtonWorld* world = World::instance->m_newtonWorld;
    int defaultID = NewtonMaterialGetDefaultGroupID(world);
    NewtonMaterialSetDefaultCollidable(world, defaultID, defaultID, 1);
  
    m_materialContact = new MaterialContact();
    m_materialContact->properties = this;
    
    NewtonMaterialSetCollisionCallback(
        world, defaultID, defaultID,
        static_cast<void*>(m_materialContact), 
        MaterialContact::onBegin, 
        MaterialContact::onProcess,
        MaterialContact::onEnd);
    
}

Properties::~Properties()
{
    delete m_materialContact;
}

int  Properties::getUndefined() const
{
    return 0;
}

int  Properties::getInvisible() const
{
    return 1;
}

int  Properties::getDefault() const
{
    return 2;
}
    
int Properties::getPropertyID(const string& name)
{
    if (name.empty())
    {
        return getDefault();
    }

    IntMap::const_iterator iter = m_propertiesID.find(name);
    if (iter == m_propertiesID.end())
    {
        int result = ++m_uniqueID;
        m_propertiesID.insert(make_pair(name, result));

        clog << "Material '" << name << "' is nr." << result << endl;
        return result;
    }
    else
    {
        return iter->second;
    }
}

bool Properties::hasPropertyID(int id) const
{
    return id >= 2;
}

void Properties::load(const XMLnode& node)
{
    NewtonWorld* world = World::instance->m_newtonWorld;

    string prop0 = getAttribute(node, "property0");
    string prop1 = getAttribute(node, "property1");
    
    const int id0 = getPropertyID(prop0);
    const int id1 = getPropertyID(prop1);
    
    if (foundInMap(m_properties, makepID(id0, id1)))
    {
        throw Exception("Properties for '" + prop0 + "' and '"
                                           + prop1 + "' already loaded");
    }

    float sF = cast<float>(getAttribute(node, "staticFriction"));
    float kF = cast<float>(getAttribute(node, "kineticFriction"));
    float eC = cast<float>(getAttribute(node, "elasticityCoeficient"));
    float sC = cast<float>(getAttribute(node, "softnessCoeficient"));

    m_properties.insert(make_pair(makepID(id0, id1), Property(sF, kF, eC, sC)));
}

void Properties::loadDefault(const XMLnode& node)
{
    NewtonWorld* world = World::instance->m_newtonWorld;

    int defaultID = NewtonMaterialGetDefaultGroupID(world);

    float sF = cast<float>(getAttribute(node, "staticFriction"));
    float kF = cast<float>(getAttribute(node, "kineticFriction"));
    float eC = cast<float>(getAttribute(node, "elasticityCoeficient"));
    float sC = cast<float>(getAttribute(node, "softnessCoeficient"));

    m_properties.insert(make_pair(makepID(getDefault(), getDefault()), Property(sF, kF, eC, sC)));

    NewtonMaterialSetDefaultElasticity(world, defaultID, defaultID, eC);
    NewtonMaterialSetDefaultFriction(world, defaultID, defaultID, sF, kF);
    NewtonMaterialSetDefaultSoftness(world, defaultID, defaultID, sC);
}

pID Properties::makepID(int id0, int id1) const
{
    if (id0 > id1)
    {
        swap(id0, id1);
    }
    // now id0 always <= id1
    return (static_cast<pID>(id0) << 32) | (id1);
}

const Property* Properties::get(int id0, int id1) const
{
    PropertiesMap::const_iterator iter = m_properties.find(makepID(id0, id1));
    
    if (iter != m_properties.end())
    {
        return &iter->second;
    }
    else
    {
        return NULL;
    }
}

int MaterialContact::onBegin(const NewtonMaterial* material, const NewtonBody* body0, const NewtonBody* body1)
{
    MaterialContact* self = static_cast<MaterialContact*>(NewtonMaterialGetMaterialPairUserData(material));
    self->body[0] = static_cast<Body*>(NewtonBodyGetUserData(body0));
    self->body[1] = static_cast<Body*>(NewtonBodyGetUserData(body1));
    
    if (body0 == body1)
    {
        assert(false);
        return 0;
    }

	self->maxNormalSpeed = 0.0f;
	self->maxTangentSpeed = 0.0f;

    return 1;
}

int MaterialContact::onProcess(const NewtonMaterial* material, const NewtonContact* contact)
{
    MaterialContact* self = static_cast<MaterialContact*>(NewtonMaterialGetMaterialPairUserData(material));
    
    int colID0 = NewtonMaterialGetBodyCollisionID(material, self->body[0]->m_newtonBody);
    int colID1 = NewtonMaterialGetBodyCollisionID(material, self->body[1]->m_newtonBody);
    
    if (colID0 == self->properties->getInvisible())
    {
        self->body[0]->onCollideHull(self->body[1], material);
        return 0;
    }
    else if (colID1 == self->properties->getInvisible())
    {
        self->body[1]->onCollideHull(self->body[0], material);
        return 0;
    }

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
       
    bool isConvex0 = self->properties->hasPropertyID(colID0);
    bool isConvex1 = self->properties->hasPropertyID(colID1);
    unsigned int faceAttr = NewtonMaterialGetContactFaceAttribute(material);

    int m0, m1;
    
    if (isConvex0 && isConvex1)
    {
        // both covex
        m0 = colID0;
        m1 = colID1;
    }
    else if (isConvex0)
    {
        m0 = colID0;
        m1 = faceAttr;
    }
    else if (isConvex1)
    {
        m0 = faceAttr;
        m1 = colID1;
    }
    else
    {
        // both not convex
        assert(false);
        // TODO: what to do?
        //return 1;
    }
    
    const Property* prop = self->properties->get(m0, m1);
    
    if (prop == NULL)
    {
        prop = self->properties->get(self->properties->getDefault(), 
                                     self->properties->getDefault());
    }

    prop->apply(material);

    return 1;
}

void MaterialContact::onEnd(const NewtonMaterial* material)
{
    MaterialContact* self = static_cast<MaterialContact*>(NewtonMaterialGetMaterialPairUserData(material));

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
