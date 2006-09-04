#ifndef __MATERIALS_H__
#define __MATERIALS_H__

#include "common.h"
#include "property.h"

class Property;
class XMLnode;

typedef long long pID;
typedef map<pID, Property> PropertiesMap;

class Properties : NoCopy
{
    friend struct MaterialContact;
public:
    Properties();
    ~Properties();

    void load(const XMLnode& node);
    void loadDefault(const XMLnode& node);
    const Property* get(int id0, int id1) const;

    int  getUndefined() const;              // 0
    int  getInvisible() const;              // 1
    int  getDefault() const;                // 2
    int  getPropertyID(const string& name); // >=3
    bool hasPropertyID(int id) const; // id>=3

private:
    int                 m_uniqueID;
    PropertiesMap       m_properties;
    IntMap              m_propertiesID;
    MaterialContact*    m_materialContact;

    pID makepID(int id0, int id1) const;
};

#endif
