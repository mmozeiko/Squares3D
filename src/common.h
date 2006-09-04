#ifndef __COMMON_H__
#define __COMMON_H__

#include <cassert>

#include <set>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#ifndef NDEBUG
#include "mmgr.h"
#endif

using std::set;
using std::map; 
using std::list;
using std::vector;

using std::string;
using std::stringstream;
using std::clog;
using std::endl;

using std::swap;
using std::pair;
using std::make_pair;

typedef map<string, string>         StringMap;
typedef set<string>                 StringSet;
typedef map<string, int>            IntMap;
typedef map<string, unsigned int>   UIntMap;
typedef pair<int, int>              IntPair;

template <typename To, typename From>
inline To cast(const From& from)
{
    stringstream ss;
    To to;
    ss << from;
    ss >> to;
    return to;
}

template <typename ValueType>
inline bool foundInVector(const vector<ValueType>& Vector,
                          const ValueType&         Value)
{
    return std::find(Vector.begin(), Vector.end(), Value) != Vector.end();
}

template <typename KeyType, typename ValueType>
inline bool foundInMap(const map<KeyType, ValueType>& Map,
                       const KeyType&                 Key)
{
    return Map.find(Key) != Map.end();
}

template <typename KeyType, typename ValueType>
inline bool foundInMap(const map<KeyType, ValueType>& Map,
                       const char*                    Key)
{
    return Map.find(Key) != Map.end();
}

template <typename KeyType>
inline bool foundInSet(const set<KeyType>& Set,
                       const KeyType&      Key)
{
    return Set.find(Key) != Set.end();
}

#define sizeOfArray(array) (sizeof(array)/sizeof((array)[0]))

#define each_(Class, Container, Iterator) \
        (Class::iterator Iterator = (Container).begin(); \
        Iterator != (Container).end(); \
        Iterator++)

#define each_const(Class, Container, Iterator) \
        (Class::const_iterator Iterator = (Container).begin(); \
        Iterator != (Container).end(); \
        Iterator++)

template <typename KeyType, typename ValueType>
inline bool foundInMapValues(const map<KeyType, ValueType>& Map,
                             const ValueType&               Value)
{
    typedef map<KeyType, ValueType> MapType;
    for each_const(MapType, Map, iter)
    {
        if (iter->second == Value)
        {
            return true;
        }
    }
    return false;
}

#define STR2(X) #X
#define STR(X) STR2(X)
#define Exception(msg) string(string(__FILE__"(") + STR(__LINE__) + ") : " + (msg))

class NoCopy
{
public:
    NoCopy() {}
private:
    NoCopy(const NoCopy&);
    NoCopy& operator = (const NoCopy&);
};

#endif
