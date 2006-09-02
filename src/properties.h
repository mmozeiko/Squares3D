#ifndef __MATERIALS_H__
#define __MATERIALS_H__

#include "common.h"
#include "property.h"

class Game;
class Property;
class XMLnode;

struct MaterialContact;
typedef set<MaterialContact*> MaterialContactSet;

typedef long long pID;
typedef map<pID, Property> PropertiesMap;

class Properties : NoCopy
{
    friend struct MaterialContact;
public:
    Properties(const Game* game);
    ~Properties();

    void load(const XMLnode& node);
    int getPropertyID(const string& name);
    const Property* get(int id0, int id1) const;

private:
    PropertiesMap       m_properties;
    MaterialContactSet  m_materialContacts;
    IntMap              m_newtonMaterials;
    const Game*         m_game;

    pID makepID(int id0, int id1) const;
};

#endif
