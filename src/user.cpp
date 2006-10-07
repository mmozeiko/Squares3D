#include "users.h"
#include "file.h"
#include "xml.h"

const string Users::USERS_FILE = "/users.xml";

Users::Users() : 
    m_active(0)
{
    clog << "Reading users information." << endl;

    XMLnode xml;
    File::Reader in(USERS_FILE);
    if (in.is_open())
    {
        xml.load(in);
        in.close();
    }

    // TODO: maybe rewrite with map<string, variable&>



    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "user")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "name")
                {
                    m_usersList.push_back(node.value);
                }
                else
                {
                    string line = cast<string>(node.line);
                    throw Exception("Invalid users file, unknown user parameter '" + node.name + "' at line " + line);
                }
            }
        }
        else
        {
            string line = cast<string>(node.line);
            throw Exception("Invalid users file, unknown section '" + node.name + "' at line " + line);
        }
    }
}

Users::~Users()
{
    clog << "Saving users data." << endl;

    XMLnode xml("users");
    for each_const(StringVector, m_usersList, iter)
    {
        xml.childs.push_back(XMLnode("user"));
        xml.childs.back().childs.push_back(XMLnode("name", *iter));
    }

    File::Writer out(USERS_FILE);
    if (!out.is_open())
    {
        throw Exception("Failed to open " + USERS_FILE + " for writing");
    }
    xml.save(out);
    out.close();
}
