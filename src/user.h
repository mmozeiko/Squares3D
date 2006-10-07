#ifndef __USER_H__
#define __USER_H__

#include "common.h"
#include "vmath.h"

class XMLnode;

class Profile : NoCopy
{
public:
    Profile(const XMLnode& node);
    ~Profile();

    string m_name;
    string m_characterType;
    Vector m_color;
};

Profile* loadUserProfile();
void saveUserProfile(const Profile* userProfile);

#endif
