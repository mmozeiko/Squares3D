#include "profile.h"
#include "file.h"
#include "xml.h"
#include "colors.h"

Profile::Profile() :
    m_name("Player"),
    m_collisionID("player_small"),
    m_color(Pink),
    m_speed(0.5f),
    m_accuracy(0.5f),
    m_jump(0.5f)
{
}

Profile::Profile(const XMLnode& node) :
    m_name("Player"),
    m_collisionID("player_small"),
    m_color(Pink),
    m_speed(0.5f),
    m_accuracy(0.5f),
    m_jump(0.5f)
{
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
            m_speed = node.getAttribute<float>("speed");
            m_jump = node.getAttribute<float>("jump");
            m_accuracy = node.getAttribute<float>("accuracy");
        }
        else
        {
            string line = cast<string>(node.line);
            throw Exception("Invalid profile file, unknown profile parameter '" + node.name + "' at line " + line);
        }
    }
}
