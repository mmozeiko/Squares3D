#include <ctime>
#include <cctype>

#include "utilities.h"

string getDateTime()
{
    time_t time;
    std::time(&time);
    
    struct tm* local = std::localtime(&time);
    
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", local);

    return buf;
}

string trim(const string& str)
{
    if (str.empty())
    {
        return str;
    }

    size_t i = 0;
    size_t j = str.size()-1;
    while (i<str.size() && std::isspace(str[i]))
    {
        i++;
    }
    while (j>i && std::isspace(str[j]))
    {
        j--;
    }
    return str.substr(i, j-i+1);
}
