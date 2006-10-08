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
    void extractNode(const XMLnode& node);
    void saveUserProfile();
    ~Profile();

    string m_name;
    string m_characterID;
    Vector m_color;
};

#endif
