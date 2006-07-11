#include "level.h"
#include "file.h"
#include "xml.h"

string getAttribute(const XMLnode& node, const string& name)
{
    StringMap::const_iterator iter = node.attributes.find(name);
    if (iter != node.attributes.end())
    {
        return node.attributes.find(name)->second;
    }

    string line = cast<int, string>(node.line);
    throw Exception("Missing attribute '" + name + "' in node '" + node.name + "' at line " + line);
}

Vector getAttributesInVector(const XMLnode& node, const string& attributeSymbols)
{
    Vector vector;
    for (size_t i = 0; i < attributeSymbols.size(); i++)
    {
        string key(1, attributeSymbols[i]);
        vector[i] = cast<string, float>(getAttribute(node, key));
    }
    return vector;

}

LevelCollision::LevelCollision(const XMLnode& node):
    m_type(),
    m_size(1.0f, 1.0f, 1.0f), 
    m_mass(1.0f),
    m_position(0.0f, 0.0f, 0.0f),
    m_rotation(0.0f, 0.0f, 0.0f),
    m_texPath(),
    m_cAmbient(0.2f, 0.2f, 0.2f),
    m_cDiffuse(0.8f, 0.8f, 0.8f),
    m_cSpecular(0.0f, 0.0f, 0.0f),
    m_cEmission(0.0f, 0.0f, 0.0f),
    m_cShine(0.0f)
{
    m_type = getAttribute(node, "type");
    
    string name = "mass";
    if (foundInMap(node.attributes, name))
    {
        m_mass = cast<string, float>(node.attributes.find(name)->second);
    }

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "size")
        {
            m_size = getAttributesInVector(node, "xyz");
        }
        else if (node.name == "offset")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "position")
                {
                    m_position = getAttributesInVector(node, "xyz");
                }
                else if (node.name == "rotation")
                {
                    m_rotation = getAttributesInVector(node, "xyz");
                }
                else
                {
                    string line = cast<int, string>(node.line);
                    throw Exception("Invalid offset, unknown node '" + node.name + "' at line " + line);
                }
            }
        }
        else if (node.name == "texture")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "path")
                {
                    m_texPath = node.value;
                }
                else
                {
                    string line = cast<int, string>(node.line);
                    throw Exception("Invalid texture, unknown node '" + node.name + "' at line " + line);
                }
            }
        }
        else if (node.name == "colors")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "ambient")
                {
                    m_cAmbient = getAttributesInVector(node, "rgb");
                }
                else if (node.name == "diffuse")
                {
                    m_cDiffuse = getAttributesInVector(node, "rgb");
                }
                else if (node.name == "specular")
                {
                    m_cSpecular = getAttributesInVector(node, "rgb");
                }
                else if (node.name == "emission")
                {
                    m_cEmission = getAttributesInVector(node, "rgb");
                }
                else if (node.name == "shine")
                {
                    m_cShine = cast<string, float>(node.value);
                }
                else
                {
                    string line = cast<int, string>(node.line);
                    throw Exception("Invalid color, unknown node '" + node.name + "' at line " + line);
                }
            }
        }
        else
        {
            string line = cast<int, string>(node.line);
            throw Exception("Invalid collision, unknown node '" + node.name + "' at line " + line);
        }
    }
}

LevelBody::LevelBody(const XMLnode& node):
    m_id(), 
    m_material(), 
    m_position(0.0f, 0.0f, 0.0f),
    m_rotation(0.0f, 0.0f, 0.0f)
{
    m_id = getAttribute(node, "id");
    m_material = getAttribute(node, "material");

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "position")
        {
            m_position = getAttributesInVector(node, "xyz");
        }
        else if (node.name == "rotation")
        {
            m_rotation = getAttributesInVector(node, "xyz");
        }
        else if (node.name == "collision")
        {
            m_collisions.insert(new LevelCollision(node));
        }
        else
        {
            string line = cast<int, string>(node.line);
            throw Exception("Invalid body, unknown node '" + node.name + "' at line " + line);
        }
    }
}

LevelBody::~LevelBody()
{
    for each_const(set<LevelCollision*>, m_collisions, iter)
    {
        delete *iter;
    }
}

Level::Level()
{
}

void Level::loadLevelData(const string& levelFile)
{
    clog << "Reading level data." << endl;

    XMLnode xml;
    File::Reader in(levelFile);
    if (!in.is_open())
    {
        throw Exception("Level file '" + levelFile + "' not found");  
    }
    in >> xml;
    in.close();

    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "bodies")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "body")
                {
                    m_bodies.insert(new LevelBody(node));
                }
                else
                {
                    string line = cast<int, string>(node.line);
                    throw Exception("Invalid body, unknown node '" + node.name + "' at line " + line);
                }
            }
        }
        else if (node.name == "materials")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "material")
                {
                    
                }
                else
                {
                    string line = cast<int, string>(node.line);
                    throw Exception("Invalid material, unknown node '" + node.name + "' at line " + line);
                }
            }
        }
        else if (node.name == "joints")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "joint")
                {

                }
                else
                {
                    string line = cast<int, string>(node.line);
                    throw Exception("Invalid joint, unknown node '" + node.name + "' at line " + line);
                }
            }
        }
        else
        {
            string line = cast<int, string>(node.line);
            throw Exception("Invalid level file, unknown section '" + node.name + "' at line " + line);
        }
    }
}

Level::~Level()
{
    for each_const(set<LevelBody*>, m_bodies, iter)
    {
        delete *iter;
    }
}
