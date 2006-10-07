#ifndef __USERS_H__
#define __USERS_H__

#include "common.h"

class Users : NoCopy
{
public:
    Users();
    ~Users();

    StringVector m_usersList;
    size_t       m_active;

private:
    static const string USERS_FILE;
};

#endif
