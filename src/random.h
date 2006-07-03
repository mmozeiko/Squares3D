#ifndef __RANDOM_H__
#define __RANDOM_H__

namespace Random
{
    void init();

    unsigned int getInt();               // [0,2^32)
    unsigned int getIntN(unsigned int n); // [0,n)
    float getFloat();                    // [0,1)
    float getFloatN(float n);             // [0,n)
}

#endif
