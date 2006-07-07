#ifndef __RANDOM_H__
#define __RANDOM_H__

namespace Random
{
    void Init();

    unsigned int GetInt();               // [0,2^32)
    unsigned int GetIntN(unsigned int n); // [0,n)
    float GetFloat();                    // [0,1)
    float GetFloatN(float n);             // [0,n)
}

#endif
