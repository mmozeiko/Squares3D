#ifndef __COMMON_H__
#define __COMMON_H__

#include <cassert>

#include <set>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <algorithm>

using std::set;
using std::map;
using std::list;
using std::vector;

using std::string;
using std::stringstream;
using std::clog;
using std::endl;

using std::pair;
using std::make_pair;

using std::auto_ptr;

typedef map<string, string> StringMap;
typedef pair<int, int> IntPair;

template <typename From, typename To>
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

template <typename KeyType>
inline bool foundInSet(const set<KeyType>& Set,
                       const KeyType&      Key)
{
    return Set.find(Key) != Set.end();
}

#define sizeOfArray(array) (sizeof(array)/sizeof((array)[0]))

#define each(Class, Container, Iterator) \
        (Class::iterator Iterator = (Container).begin(); \
        Iterator != (Container).end(); \
        Iterator++)

#define each_const(Class, Container, Iterator) \
        (Class::const_iterator Iterator = (Container).begin(); \
        Iterator != (Container).end(); \
        Iterator++)

#define STR2(X) #X
#define STR(X) STR2(X)
#define Exception(msg) string(string(__FILE__"(") + STR(__LINE__) + ") : " + (msg))

#endif
