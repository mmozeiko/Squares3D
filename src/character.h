#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include "vmath.h"

class Level;
class XMLnode;
class Body;

//this class is supposed to hold all unique properties of player

class Character
{
public:
    Character(const XMLnode& node, const Level* level);
    void setNameAndColor(const string& name, const Vector& color) { m_name = name; m_color = color; }
    void loadBody(Level* level);

    Body*   m_body;
    float   m_speed;
    float   m_accuracy;
    string  m_type;
    string  m_collisionID;
    string  m_name;
    Vector  m_color;
};


#endif
