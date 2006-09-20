#ifndef __XML_H__
#define __XML_H__

#include "common.h"
#include "file.h"

class XMLnode;

typedef vector<XMLnode> XMLnodes;

class XMLreader;
class XMLwriter;

class XMLnode
{
    friend void outputNode(const XMLnode& node, std::ostream& stream, int level);
    friend class XMLreader;
    friend class XMLwriter;
public:
    XMLnode() {};
    XMLnode(const string& name, const string& value = "")
        : name(name), value(value), line(0) {}

    void load(File::Reader& reader);
    void save(File::Writer& writer);

    XMLnodes childs;
    string name;
    string value;
    unsigned int line;

    inline bool hasAttributes() const;

    inline bool hasAttribute(const string& name) const;

    template <typename T>
    inline T getAttribute(const string& name) const;

    inline string getAttribute(const string& name) const;

    template <typename T>
    inline T getAttribute(const string& name, T defaultValue) const;

    inline string getAttribute(const string& name, const string& defaultValue) const;

private:
    StringMap attributes;
};

inline bool XMLnode::hasAttributes() const
{
    return attributes.size() != 0;
}

inline bool XMLnode::hasAttribute(const string& name) const
{
    return foundInMap(attributes, name);
}

template <typename T>
T XMLnode::getAttribute(const string& name) const
{
    StringMap::const_iterator iter = attributes.find(name);
    if (iter != attributes.end())
    {
        return cast<T>(iter->second);
    }

    throw Exception("Missing attribute '" + name + "' in node '" + name + "'");
}

string XMLnode::getAttribute(const string& name) const
{
    StringMap::const_iterator iter = attributes.find(name);
    if (iter != attributes.end())
    {
        return iter->second;
    }

    throw Exception("Missing attribute '" + name + "' in node '" + name + "'");
}

template <typename T>
T XMLnode::getAttribute(const string& name, T defaultValue) const
{
    StringMap::const_iterator iter = attributes.find(name);
    if (iter != attributes.end())
    {
        return cast<T>(iter->second);
    }
    return defaultValue;
}

string XMLnode::getAttribute(const string& name, const string& defaultValue) const
{
    StringMap::const_iterator iter = attributes.find(name);
    if (iter != attributes.end())
    {
        return iter->second;
    }
    return defaultValue;
}

#endif // __XML_H__
