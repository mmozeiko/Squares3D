#ifndef __PROFILE_H__
#define __PROFILE_H__

#include "common.h"
#include "vmath.h"

class XMLnode;

class Profile : NoCopy
{
public:
    Profile(const XMLnode& node);
    Profile();
    Profile(const Profile& profile);

    string m_name;
    string m_collisionID;
    Vector m_color;
    float  m_speed;
    float  m_accuracy;
    float  m_jump;
};

#endif
