#include "user.h"
#include "file.h"
#include "xml.h"

const static string USER_PROFILE_FILE = "/user.xml";

Profile* loadUserProfile()
{
    clog << "Reading user information." << endl;
    XMLnode xml;
    File::Reader in(USER_PROFILE_FILE);
    if (in.is_open())
    {
        xml.load(in);
        in.close();
    }
    return new Profile(xml);
}


Profile::Profile(const XMLnode& node)
{
    // TODO: maybe rewrite with map<string, variable&>

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "name")
        {
            m_name = node.value;
        }
        else if (node.name == "character")
        {
            m_characterType = node.value;
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

void saveUserProfile(const Profile* userProfile)
{
    clog << "Saving user data." << endl;

    XMLnode xml("profile");
    xml.childs.push_back(XMLnode("name", userProfile->m_name));
    xml.childs.push_back(XMLnode("character", userProfile->m_characterType));
    xml.childs.push_back(XMLnode("color"));
    xml.childs.back().setAttribute("r", cast<string>(userProfile->m_color.x));
    xml.childs.back().setAttribute("g", cast<string>(userProfile->m_color.y));
    xml.childs.back().setAttribute("b", cast<string>(userProfile->m_color.z));

    File::Writer out(USER_PROFILE_FILE);
    if (!out.is_open())
    {
        throw Exception("Failed to open " + USER_PROFILE_FILE + " for writing");
    }
    xml.save(out);
    out.close();
    
    delete userProfile;
}
