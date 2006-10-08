#include "profile.h"
#include "file.h"
#include "xml.h"

const static string USER_PROFILE_FILE = "/user.xml";

Profile::Profile()
{
    clog << "Reading user information." << endl;
    XMLnode xml;
    File::Reader in(USER_PROFILE_FILE);
    if (in.is_open())
    {
        xml.load(in);
        in.close();
    }
    extractNode(xml);
}


Profile::Profile(const XMLnode& node)
{
    // TODO: maybe rewrite with map<string, variable&>

    extractNode(node);
}

void Profile::extractNode(const XMLnode& node)
{
    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "name")
        {
            m_name = node.value;
        }
        else if (node.name == "character")
        {
            m_characterID = node.value;
        }
        else if (node.name == "color")
        {
            m_color = node.getAttributesInVector("rgb");
        }
        else
        {
            string line = cast<string>(node.line);
            throw Exception("Invalid profile file, unknown profile parameter '" + node.name + "' at line " + line);
        }
    }
}

Profile::~Profile()
{
}

void Profile::saveUserProfile()
{
    clog << "Saving user data." << endl;

    XMLnode xml("profile");
    xml.childs.push_back(XMLnode("name", m_name));
    xml.childs.push_back(XMLnode("character", m_characterID));
    xml.childs.push_back(XMLnode("color"));
    xml.childs.back().setAttribute("r", cast<string>(m_color.x));
    xml.childs.back().setAttribute("g", cast<string>(m_color.y));
    xml.childs.back().setAttribute("b", cast<string>(m_color.z));

    File::Writer out(USER_PROFILE_FILE);
    if (!out.is_open())
    {
        throw Exception("Failed to open " + USER_PROFILE_FILE + " for writing");
    }
    xml.save(out);
    out.close();
}
