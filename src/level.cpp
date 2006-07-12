#include "level.h"
#include "file.h"
#include "xml.h"
#include "video.h"
#include "game.h"

string getAttribute(const XMLnode& node, const string& name)
{
    if (foundInMap(node.attributes, name))
    {
        return node.attributes.find(name)->second;
    }

    throw Exception("Missing attribute '" + name + "' in node '" + node.name + "'");
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

using namespace LevelObjects;

Material::Material(const XMLnode& node, const Game* game) :
    m_id(),
    m_texPath(),
    m_cAmbient(0.2f, 0.2f, 0.2f),
    m_cDiffuse(0.8f, 0.8f, 0.8f),
    m_cSpecular(0.0f, 0.0f, 0.0f),
    m_cEmission(0.0f, 0.0f, 0.0f),
    m_cShine(0.0f),
    m_texture(0)
{
    m_id = getAttribute(node, "id");

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "texture")
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
                    throw Exception("Invalid texture, unknown node - " + node.name);
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
                    throw Exception("Invalid color, unknown node - " + node.name);
                }
            }
        }
        else
        {
            throw Exception("Invalid material, unknown node - " + node.name);
        }
    }
    m_texture = game->m_video->loadTexture(m_texPath);
}

void Material::render(const Video* video) const
{
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_cAmbient.v);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_cDiffuse.v);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_cSpecular.v);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, m_cEmission.v);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_cShine);
}

Level::Level(const Game* game) :
    m_game(game)
{
}

void Level::load(const string& levelFile)
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
                    m_bodies.insert(new Body(node));
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
                    m_materials[getAttribute(node, "id")] = new Material(node, m_game);
                }
                else if (node.name == "properties")
                {
                    
                }
                else
                {
                    throw Exception("Invalid materials, unknown node - " + node.name);
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
                    throw Exception("Invalid joint, unknown node - " + node.name);
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
    for each_(set<Body*>, m_bodies, iter)
    {
        delete *iter;
    }
    for each_(MaterialsMap, m_materials, iter)
    {
        delete iter->second;
    }
}

void Level::render(const Video* video) const
{
    // TODO: Remove
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);

    glPushAttrib(GL_LIGHTING_BIT);
    for each_const(set<Body*>, m_bodies, iter)
    {
       (*iter)->render(video, &m_materials);
    }    
    glPopAttrib();
    
    // TODO: Remove
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);
}

Body::Body(const XMLnode& node):
    m_id(), 
    m_material(), 
    m_position(0.0f, 0.0f, 0.0f),
    m_rotation(0.0f, 0.0f, 0.0f)
{
    m_id = getAttribute(node, "id");

    string name = "material";
    if (foundInMap(node.attributes, name))
    {
        m_material = getAttribute(node, name);
    }

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
            m_collisions.insert(Collision::create(node));
        }
        else
        {
            throw Exception("Invalid body, unknown node - " + node.name);
        }
    }
}

Body::~Body()
{
    for each_(set<Collision*>, m_collisions, iter)
    {
        delete *iter;
    }
}

void Body::render(const Video* video, const MaterialsMap* materials) const
{
    MaterialsMap::const_iterator material = materials->find(m_material);
    if (material != materials->end())
    {
        material->second->render(video);
    }

    for each_const(set<Collision*>, m_collisions, iter)
    {
        (*iter)->render(video, materials);
    }
}

Collision::Collision(const XMLnode& node)
{
}

CollisionBox::CollisionBox(const XMLnode& node) :
    Collision(node),
    m_size(1.0f, 1.0f, 1.0f), 
    m_mass(1.0f),
    m_position(0.0f, 0.0f, 0.0f),
    m_rotation(0.0f, 0.0f, 0.0f)
{
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
        else if (node.name == "position")
        {
            m_position = getAttributesInVector(node, "xyz");
        }
        else if (node.name == "rotation")
        {
            m_rotation = getAttributesInVector(node, "xyz");
        }
        else
        {
            throw Exception("Invalid collision, unknown node - " + node.name);
        }
    }
}

void CollisionBox::render(const Video* video, const MaterialsMap* materials) const
{   
    glPushMatrix();
    glTranslatef(m_position.x, m_position.y, m_position.z);
    glScalef(m_size.x, m_size.y, m_size.z);
    video->renderCube();
    glPopMatrix();
}

Collision* Collision::create(const XMLnode& node)
{
    string type = getAttribute(node, "type");
    if (type == "box")
    {
        return new CollisionBox(node);
    }
    else if (type == "tree")
    {
        return new CollisionTree(node);
    }
    else
    {
        throw Exception("Unknown collision type - " + type);
    }
}

CollisionTree::CollisionTree(const XMLnode& node) :
    Collision(node)
{
    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "face")
        {
            m_materials.push_back(getAttribute(node, "material"));
            m_faces.push_back(Face());
            Face& face = m_faces.back();
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "vertex")
                {
                    face.push_back(getAttributesInVector(node, "xyz"));
                }
                else
                { 
                    throw Exception("Invalid face, unknown node - " + node.name);
                }
            }
        }
        else
        {
            throw Exception("Invalid collision, unknown node - " + node.name);
        }
    }
}

void CollisionTree::render(const Video* video, const MaterialsMap* materials) const
{
    //clog << m_faces[0][0].v[0] << endl;
    for (size_t i = 0; i < m_faces.size(); i++)
    {
        materials->find(m_materials[i])->second->render(video);
        vector<vector<int>> uv;

        vector<int> int2;
        int2.push_back(0), int2.push_back(0); uv.push_back(int2); int2.clear();
        int2.push_back(0), int2.push_back(1); uv.push_back(int2); int2.clear();
        int2.push_back(1), int2.push_back(1); uv.push_back(int2); int2.clear();
        int2.push_back(1), int2.push_back(0); uv.push_back(int2);

        video->renderFace(m_faces[i], uv);
    }
}
