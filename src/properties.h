#ifndef __MATERIALS_H__
#define __MATERIALS_H__

#include "common.h"
#include "property.h"
#include "vmath.h"

class Property;
class XMLnode;
class Sound;
class SoundBuffer;
class Body;
struct MaterialContact;

typedef long long pID;
typedef map<pID, Property> PropertiesMap;

struct Soundable
{
    Soundable(Sound* sound, bool important) : body(NULL), sound(sound), important(important) {}

    Body*  body;
    Sound* sound;
    bool   important;
};

typedef list<Soundable>                   SoundableList;
typedef vector<pair<byte, SoundBuffer*> > SoundBufferVector;
typedef map<pID, SoundBufferVector>       SoundBufMap;

class Properties : public NoCopy
{
    friend struct MaterialContact;
public:
    Properties();
    ~Properties();

    void update();

    void load(const XMLnode& node);
    void loadDefault(const XMLnode& node);
    const Property* get(int id0, int id1) const;
    const pair<byte, SoundBuffer*>* getSB(int id0, int id1) const;

    int  getUndefined() const;              // 0
    int  getInvisible() const;              // 1
    int  getDefault() const;                // 2
    int  getPropertyID(const string& name); // >=3
    int  getPropertyID(const string& name) const; // >=3
    bool hasPropertyID(int id) const; // id>=3

    void play(Body* body, const pair<byte, SoundBuffer*>* buffer, bool important, const Vector& position);
    bool isPlaying(Body* body) const;

    void play(byte id, const Vector& position);

private:
    int                 m_uniqueID;
    PropertiesMap       m_properties;
    SoundBufMap         m_soundBufs;
    byte                m_soundBufID;
    IntMap              m_propertiesID;
    MaterialContact*    m_materialContact;

    pID makepID(int id0, int id1) const;

    SoundableList m_active;
    SoundableList m_idle;
};

#endif
