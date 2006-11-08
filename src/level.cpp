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
#include "music.h"
#include "audio.h"
#include "config.h"

Level::Level() : m_gravity(0.0f, -9.81f, 0.0f), m_skyboxName(),
    m_hdr_eps(0.60f), m_hdr_exp(-0.35f), m_hdr_mul(1.0f, 1.0f, 0.8f, 1.0f)
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

    if (foundIn(loaded, levelFile))
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
        if (node.name == "hdr")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "eps")
                {
                    m_hdr_eps = node.getAttribute<float>("value");
                }
                else if (node.name == "exp")
                {
                    m_hdr_exp = node.getAttribute<float>("value");
                }
                else if (node.name == "mul")
                {
                    m_hdr_mul = node.getAttributesInVector("rgb");
                }
                else
                {
                    throw Exception("Invalid hdr, unknown node - " + node.name);
                }
            }
        }
        else if (node.name == "gravity")
        {
            m_gravity = node.getAttributesInVector("xyz");
        }
        else if (node.name == "music")
        {
            m_music.push_back(Audio::instance->loadMusic(node.getAttribute("name")));
        }
        else if (node.name == "skybox")
        {
            m_skyboxName = node.getAttribute("name");
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
    if (m_skyboxName.empty())
    {
        throw Exception("Skybox not specified in level file");
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

    if (!m_music.empty())
    {
        m_music[0]->stop();
        for each_const(MusicVector, m_music, iter)
        {
            Audio::instance->unloadMusic(*iter);
        }
    }
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
    m_materials.find("penguin")->second->bind();

    for each_const(BodiesMap, m_bodies, iter)
    {
        if (Config::instance->m_video.use_hdr && iter->second->m_id == "field")
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-1.0f, -1.0f);
        }
        (iter->second)->render();
        if (Config::instance->m_video.use_hdr && iter->second->m_id == "field")
        {
            glDisable(GL_POLYGON_OFFSET_FILL);
        }
    }    
}
