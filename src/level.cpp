#include "level.h"
#include "file.h"
#include "xml.h"
#include "video.h"
#include "world.h"
#include "material.h"
#include "collision.h"
#include "body.h"
#include "properties.h"

Level::Level()
{
    m_properties = new Properties();
}

void Level::load(const string& levelFile, StringSet& loaded)
{
    clog << "Reading '" << levelFile << "' data." << endl;

    if (foundInSet(loaded, levelFile))
    {
        clog << "ERROR: " << levelFile << " already is loaded!" << endl;
        return;
    }
    loaded.insert(levelFile);

    XMLnode xml;
    File::Reader in("/data/level/" + levelFile);
    if (!in.is_open())
    {
        throw Exception("Level file '" + levelFile + "' not found");  
    }
    xml.load(in);
    in.close();

    for each_const(XMLnodes, xml.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "link")
        {
            load(getAttribute(node, "file"), loaded);
        }
        else if (node.name == "bodies")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "body")
                {
                    string id = getAttribute(node, "id");
                    m_bodies[id] = new Body(node);
                }
                else
                {
                    throw Exception("Invalid body, unknown node - " + node.name);
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
                    m_materials.insert(make_pair(getAttribute(node, "id"), new Material(node)));
                }
                else
                {
                    throw Exception("Invalid materials, unknown node - " + node.name);
                }
            }
        }
        else if (node.name == "collisions")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "collision")
                {
                    m_collisions[getAttribute(node, "id")] = Collision::create(node, this);
                }
                else
                {
                    throw Exception("Invalid collisions, unknown node - " + node.name);
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
                    // TODO: load joints
                }
                else
                {
                    throw Exception("Invalid joint, unknown node - " + node.name);
                }
            }
        }
        else if (node.name == "properties")
        {
            m_properties->load(node);
        }
        else if (node.name == "defaultProperties")
        {
            m_properties->loadDefault(node);
        }
        else
        {
            throw Exception("Invalid level file, unknown section - " + node.name);
        }
    }
}

Level::~Level()
{
    for each_(BodiesMap, m_bodies, iter)
    {
        delete iter->second;
    }
    for each_const(CollisionsMap, m_collisions, iter)
    {
        delete iter->second;
    }
    for each_const(MaterialMap, m_materials, iter)
    {
        delete iter->second;
    }
    delete m_properties;
}

Body* Level::getBody(const string id)
{
    BodiesMap::iterator body = m_bodies.find(id);
    if (body != m_bodies.end())
    {
        return body->second;
    }
    else
    {
        throw Exception("Couldn`t load body '" + id + "'");
    }
}

void Level::prepare()
{
    for each_const(BodiesMap, m_bodies, iter)
    {
        (iter->second)->prepare();
    }
}

void Level::render() const
{
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_COLOR_MATERIAL);
    for each_const(BodiesMap, m_bodies, iter)
    {
        if (iter->first != "invisibleWalls") // TODO: hack
        {
            (iter->second)->render();
        }
    }    
    glPopAttrib();
}

string getAttribute(const XMLnode& node, const string& name)
{
    StringMap::const_iterator iter = node.attributes.find(name);
    if (iter != node.attributes.end())
    {
        return iter->second;
    }

    throw Exception("Missing attribute '" + name + "' in node '" + node.name + "'");
}

string getAttribute(const XMLnode& node, const string& name, const string& defaultValue)
{
    StringMap::const_iterator iter = node.attributes.find(name);
    if (iter != node.attributes.end())
    {
        return iter->second;
    }
    return defaultValue;
}

Vector getAttributesInVector(const XMLnode& node, const string& attributeSymbols)
{
    Vector vector;
    for (size_t i = 0; i < attributeSymbols.size(); i++)
    {
        string key(1, attributeSymbols[i]);
        vector[i] = cast<float>(getAttribute(node, key));
    }
    return vector;

}
