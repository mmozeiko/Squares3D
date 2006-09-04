#include <enet/enet.h>

#include "network.h"
#include "random.h"

Network* System<Network>::instance = NULL;

Network::Network()
{
    clog << "Initializing network." << endl;

    ENetCallbacks callbacks = { 
        NULL, NULL,                                                     // malloc, free
        reinterpret_cast<int (ENET_CALLBACK*)(void)>(Random::getInt)    // rand
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
