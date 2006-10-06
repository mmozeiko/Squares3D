#include <Newton.h>
#include "character.h"
#include "body.h"
#include "collision.h"
#include "xml.h"
#include "level.h"


Character::Character(const XMLnode& node, const Level* level) : 
    m_color(Vector::One),
    m_speed(0.1f),
    m_accuracy(0.1f)
{
    m_name = node.getAttribute("id");

    Collision* collision = level->getCollision(node.getAttribute("collisionID"));
    
    m_body = new Body(m_name, collision);

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "color")
        {
            m_color = getAttributesInVector(node, "rgb");
        }
        else if (node.name == "char")
        {
            m_speed = node.getAttribute<float>("speed");
            m_accuracy = node.getAttribute<float>("accuracy");
        }
        else
        {
            throw Exception("Invalid player, unknown node - " + node.name);
        }
    }
}
