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

bool is_space(char c)
{
    return c=='\n' || c=='\t' || c==' ' || c=='\r';
}

string trim(const string& str)
{
    if (str.empty())
    {
        return str;
    }

    size_t i = 0;
    size_t j = str.size()-1;
    while (i<str.size() && str[i]>=0 && str[i]<128 && is_space(str[i]))
    {
        i++;
    }
    while (j>i && str[i]>=0 && str[i]<128 && is_space(str[j]))
    {
        j--;
    }
    return str.substr(i, j-i+1);
}
