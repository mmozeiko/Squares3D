#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "common.h"
#include "system.h"

class Network : public System<Network>, NoCopy
{
public:
    Network();
    ~Network();

private:
    //
};

#endif
