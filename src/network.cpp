#include "network.h"
#include "random.h"
#include "common.h"

#include <enet/enet.h>

Network::Network()
{
    clog << "Initializing network." << endl;

    ENetCallbacks callbacks = { 
        NULL, NULL,                                                     // malloc, free
        reinterpret_cast<int (ENET_CALLBACK*)(void)>(Random::GetInt)    // rand
    };

    if (enet_initialize_with_callbacks(ENET_VERSION, &callbacks) != 0)
    {
        throw Exception("enet_initialize failed");
    }
}


Network::~Network()
{
    clog << "Closing network." << endl;
    enet_deinitialize();
}
