#include "profile.h"
#include "file.h"
#include "colors.h"
#include "xml.h"

const static string USER_PROFILE_FILE = "/user.xml";

Profile::Profile() :
    m_speed(0.1f),
    m_accuracy(0.1f),
    m_jump(0.1f)
{
    clog << "Reading user information." << endl;
    XMLnode xml;
    File::Reader in(USER_PROFILE_FILE);
    if (in.is_open())
    {
        xml.load(in);
        in.close();
        extractNode(xml);
    }
    else
    {
        loadProfileWithDefaultValues();
    }
    
}

void Profile::loadProfileWithDefaultValues()
{
    m_name = "UnamedPlayer";
    m_collisionID = "player_small";
    m_color = Pink;
    m_speed = 0.5f;
    m_accuracy = 0.5f;
    m_jump = 0.5f;
}

Profile::Profile(const XMLnode& node) :
    m_speed(0.1f),
    m_accuracy(0.1f),
    m_jump(0.1f)
{
    extractNode(node);
}

void Profile::extractNode(const XMLnode& node)
{
    // TODO: maybe rewrite with map<string, variable&>
    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "name")
        {
            m_name = node.value;
        }
        else if (node.name == "collision")
        {
            m_collisionID = node.value;
        }
        else if (node.name == "color")
        {
            m_color = node.getAttributesInVector("rgb");
        }
        else if (node.name == "properties")
        {
            m_speed = cast<float>(node.getAttribute("speed"));
            m_jump = cast<float>(node.getAttribute("jump"));
            m_accuracy = cast<float>(node.getAttribute("accuracy"));
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
    xml.childs.push_back(XMLnode("collision", m_collisionID));
    xml.childs.push_back(XMLnode("properties"));
    xml.childs.back().setAttribute("speed", cast<string>(m_speed));
    xml.childs.back().setAttribute("accuracy", cast<string>(m_accuracy));
    xml.childs.back().setAttribute("jump", cast<string>(m_jump));
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
