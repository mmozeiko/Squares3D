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
    //there probably will be a need to make another constructor for Character
    //which initializes the object with character data recieved from network
    //and makes the copies (and appends them to level) of m_body,
    //collision of this body and tells Newton to create them
    //like Character(const Character* character) {..do these things..}

    Character(const XMLnode& node, const Level* level);

    Body*   m_body;
    Vector  m_color;
    float   m_speed;
    float   m_accuracy;
    string  m_name;
};


#endif
