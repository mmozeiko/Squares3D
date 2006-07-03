#include "utilities.h"

#include <ctime>

string getDateTime()
{
    time_t time;
    std::time(&time);
    
    struct tm* local = std::localtime(&time);
    
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", local);

    return buf;
}
