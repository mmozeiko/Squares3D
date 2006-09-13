#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "common.h"

template <class T>
class System
{
public:
    static T* instance;
    
    System()
    {
        assert(instance == NULL);
        instance = reinterpret_cast<T*>(this);
    }

    ~System()
    {
        assert(instance != NULL);
        instance = NULL;
    }

    void setInstance(T* instance)
    {
        this->instance = instance;
    }
};

#endif