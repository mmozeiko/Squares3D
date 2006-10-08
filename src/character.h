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

    float   m_speed;
    float   m_accuracy;
    string  m_collisionID;
};


#endif
