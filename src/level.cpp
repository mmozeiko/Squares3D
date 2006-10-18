#include "level.h"
#include "file.h"
#include "xml.h"
#include "video.h"
#include "world.h"
#include "material.h"
#include "collision.h"
#include "body.h"
#include "properties.h"
#include "profile.h"

Level::Level() : m_gravity(0.0f, -9.81f, 0.0f)
{
    m_properties = new Properties();
}

void Level::load(const string& levelFile)
{
    StringSet tmp;
    load(levelFile, tmp);
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
        if (node.name == "gravity")
        {
            m_gravity = node.getAttributesInVector("xyz");
        }
        else if (node.name == "link")
        {
            load(node.getAttribute("file"), loaded);
        }
        else if (node.name == "bodies")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "body")
                {
                    string id = node.getAttribute("id");
                    m_bodies[id] = new Body(node, this);
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
                    m_materials.insert(make_pair(node.getAttribute("id"), new Material(node)));
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
                    m_collisions[node.getAttribute("id")] = Collision::create(node, this);
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
        else if (node.name == "fences")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "fence")
                {
                    vector<Vector> points;
                    for each_const(XMLnodes, node.childs, iter)
                    {
                        const XMLnode& node = *iter;
                        if (node.name == "point")
                        {
                            points.push_back(node.getAttributesInVector("xyz"));
                        }
                        else
                        {
                            throw Exception("Invalid fence, unknown node - " + node.name);
                        }
                    }
                    if (points.size() < 2)
                    {
                        throw Exception("Too less points in fence, expected at least 2");
                    }
                    m_fences.push_back(points);
                }
                else
                {
                    throw Exception("Invalid fences, unknown node - " + node.name);
                }
            }
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
    for each_const(MaterialsMap, m_materials, iter)
    {
        delete iter->second;
    }
    delete m_properties;
}

Body* Level::getBody(const string& id) const
{
    BodiesMap::const_iterator body = m_bodies.find(id);
    if (body != m_bodies.end())
    {
        return body->second;
    }
    else
    {
        throw Exception("Couldn`t load body '" + id + "'");
    }
}

Collision* Level::getCollision(const string& id) const
{
    CollisionsMap::const_iterator iter = m_collisions.find(id);
    if (iter != m_collisions.end())
    {
        return iter->second;
    }
    else
    {
        throw Exception("Could not find specified collision '" + id + "'");
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
    for each_const(BodiesMap, m_bodies, iter)
    {
        (iter->second)->render();
    }    
}
