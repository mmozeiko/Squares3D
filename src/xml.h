#ifndef __XML_H__
#define __XML_H__

#include <iosfwd>
#include "common.h"

class XMLnode;
typedef vector<XMLnode> XMLnodes;
class XMLnode
{
    friend std::ostream& operator << (std::ostream& stream, const XMLnode& xml);
    friend std::istream& operator >> (std::istream& stream, XMLnode& xml);

public:
    StringMap attributes;
    XMLnodes childs;
    string name;
    string value;
};

#endif // __XML_H__
