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

#if defined(_DEBUG) && !__MINGW32__ && !__APPLE__ && !__linux__
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

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned int   uint;

typedef vector<byte> bytes;

typedef map<string, string> StringMap;
typedef set<string>         StringSet;
typedef vector<string>      StringVector;
typedef map<string, int>    IntMap;
typedef map<string, uint>   UIntMap;
typedef pair<int, int>      IntPair;
typedef set<IntPair>        IntPairSet;
typedef vector<IntPair>     IntPairVector;
typedef set<uint>           UIntSet;
typedef set<int>            IntSet;
typedef vector<int>         IntVector;
typedef list<int>           IntList;

template <typename To, typename From>
inline To cast(const From& from)
{
    stringstream ss;
    To to;
    ss << from;
    ss >> to;
    return to;
}

template <>
inline string cast(const string& from)
{
    return from;
}

template <typename ValueType>
inline bool foundIn(const vector<ValueType>& Vector,
                    ValueType                Value)
{
    return std::find(Vector.begin(), Vector.end(), Value) != Vector.end();
}

template <typename KeyType, typename ValueType>
inline bool foundIn(const map<KeyType, ValueType>& Map,
                    KeyType                        Key)
{
    return Map.find(Key) != Map.end();
}

template <typename KeyType, typename ValueType>
inline bool foundIn(const map<KeyType, ValueType>& Map,
                    const char*                    Key)
{
    return Map.find(Key) != Map.end();
}

template <typename KeyType>
inline bool foundIn(const set<KeyType>& Set,
                    KeyType             Key)
{
    return Set.find(Key) != Set.end();
}

template <typename ArrayType, int N>
inline bool foundIn(const ArrayType arr[N],
                    ArrayType       Value)
{
    return std::find(arr, arr + N, Value) != arr + N;
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

#define each_constT(Class, Container, Iterator) \
        (typename Class::const_iterator Iterator = (Container).begin(); \
        Iterator != (Container).end(); \
        Iterator++)

template <typename KeyType, typename ValueType>
inline bool foundInMapValues(const map<KeyType, ValueType>& Map,
                             const ValueType&               Value)
{
    typedef map<KeyType, ValueType> MapType;
    for each_constT(MapType, Map, iter)
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
#define Exception(msg) string(string(__FILE__"(") + STR(__LINE__) + ") : " + string(__FUNCTION__) + " - " + string(msg))

class NoCopy
{
public:
    NoCopy() {}
protected:
    NoCopy(const NoCopy&);
    NoCopy& operator = (const NoCopy&);
};

template<bool> struct CompileTimeError;
template<> struct CompileTimeError<true> {};

#define STATIC_CHECK(check, msg) \
    { CompileTimeError<(check)> ERROR_##msg; ERROR_##msg; }

#endif
