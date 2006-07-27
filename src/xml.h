#ifndef __XML_H__
#define __XML_H__

#include "file.h"
#include "common.h"

class XMLnode;
typedef vector<XMLnode> XMLnodes;
class XMLnode
{
public:
    XMLnode() {};
    XMLnode(const string& name, const string& value = "")
        : name(name), value(value), line(0) {}

    void load(File::Reader& reader);
    void save(File::Writer& writer);

    StringMap attributes;
    XMLnodes childs;
    string name;
    string value;
    unsigned int line;
};

#endif // __XML_H__
