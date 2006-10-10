#ifndef __PROFILE_H__
#define __PROFILE_H__

#include "common.h"
#include "vmath.h"

class XMLnode;

class Profile : NoCopy
{
public:
    Profile();
    Profile(const XMLnode& node);

    void saveUserProfile();
    ~Profile();

    string m_name;
    string m_collisionID;
    Vector m_color;
    float  m_speed;
    float  m_accuracy;
    float  m_jump;

private:
    void loadProfileWithDefaultValues();
    void extractNode(const XMLnode& node);
};

#endif
