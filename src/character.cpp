#include "character.h"
#include "xml.h"
#include "level.h"


Character::Character(const XMLnode& node, const Level* level) : 
    m_speed(0.1f),
    m_accuracy(0.1f)
{
    m_collisionID = node.getAttribute("collisionID");

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "properties")
        {
            m_speed = node.getAttribute<float>("speed");
            m_accuracy = node.getAttribute<float>("accuracy");
        }
        else
        {
            throw Exception("Invalid character, unknown node - " + node.name);
        }
    }
}
