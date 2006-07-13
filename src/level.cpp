#include "level.h"
#include "file.h"
#include "xml.h"
#include "video.h"
#include "game.h"
#include "world.h"

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

void __onSetForceAndTorque(const NewtonBody* m_newtonBody)
{
    float mass;
    float Ixx, Iyy, Izz;

    // Get the mass of the object
    NewtonBodyGetMassMatrix(m_newtonBody, &mass, &Ixx, &Iyy, &Izz );
    
    Vector force = gravityVec * mass;

    NewtonBodyAddForce(m_newtonBody, force.v);

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
    video->applyTexture(m_texture);

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
                    m_bodies.insert(new Body(node, m_game));
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

void Level::prepare()
{
    for each_const(set<LevelObjects::Body*>, m_bodies, iter)
    {
        (*iter)->prepare();
    }
}

void Level::render(const Video* video) const
{
    // TODO: Remove
    glEnable(GL_TEXTURE_2D);

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_COLOR_MATERIAL);
    for each_const(set<Body*>, m_bodies, iter)
    {
       (*iter)->render(video, &m_materials);
    }    
    glPopAttrib();
    
    // TODO: Remove
    glDisable(GL_TEXTURE_2D);
}

Body::Body(const XMLnode& node, const Game* game):
    m_id(), 
    m_material(), 
    m_position(0.0f, 0.0f, 0.0f),
    m_rotation(0.0f, 0.0f, 0.0f),
    m_matrix()
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
            m_collisions.insert(Collision::create(node, game));
        }
        else
        {
            throw Exception("Invalid body, unknown node - " + node.name);
        }
    }

    static int i = 0;
    i++;

    //right now we support just one collision per body
    if (m_collisions.size() == 1)
    {
        NewtonCollision* collision = (*m_collisions.begin())->m_newtonCollision;
        m_newtonBody = NewtonCreateBody(game->m_world->m_world, collision);
        if (i==2)
        {
            NewtonBodySetMassMatrix(m_newtonBody, 10.0f, 1, 1, 1);
        }
        
        NewtonBodySetForceAndTorqueCallback(m_newtonBody, __onSetForceAndTorque);

        m_matrix = m_matrix.identity();
        //NewtonSetEulerAngle(m_rotation.v, m_matrix.m);
        m_matrix = m_matrix * Matrix::translate(m_position);
                
        NewtonBodySetMatrix(m_newtonBody, m_matrix.m);

        NewtonBodySetAutoFreeze(m_newtonBody, 0);
	    NewtonWorldUnfreezeBody(game->m_world->m_world, m_newtonBody);


     //   NewtonBodySetUserData(m_body, static_cast<void*>(this));

        NewtonReleaseCollision(game->m_world->m_world, collision);
     //   
	    //// set the viscous damping the the minimum
     //   const float damp[] = { 0.0f, 0.0f, 0.0f };
	    //NewtonBodySetLinearDamping(m_body, 0.0f);
	    //NewtonBodySetAngularDamping(m_body, damp);

	    //// Set Material Id for this object
	    //NewtonBodySetMaterialGroupID(m_body, material);
    }

}

Body::~Body()
{
    for each_(set<Collision*>, m_collisions, iter)
    {
        delete *iter;
    }
}

void Body::prepare()
{
    NewtonBodyGetMatrix(m_newtonBody, m_matrix.m);
}

void Body::onSetForceAndTorque()
{
    float mass;
    float Ixx, Iyy, Izz;

    // Get the mass of the object
    NewtonBodyGetMassMatrix(m_newtonBody, &mass, &Ixx, &Iyy, &Izz );
    
    Vector force = gravityVec * mass;

    NewtonBodyAddForce(m_newtonBody, force.v);

}

void Body::render(const Video* video, const MaterialsMap* materials)
{
    Vector force(10,3,0);
    MaterialsMap::const_iterator material = materials->find(m_material);
    if (material != materials->end())
    {
        material->second->render(video);
    }

    for each_const(set<Collision*>, m_collisions, iter)
    {
        //set the rotation and transformation offset of collision
        //NewtonSetEulerAngle(m_rotation.v, m_matrix.m);
        //m_matrix = m_matrix * Matrix::translate(m_position);
        video->begin(m_matrix.m);
        (*iter)->render(video, materials);
        video->end();
    }
}

Collision::Collision(const XMLnode& node, const Game* game)
{
}

CollisionBox::CollisionBox(const XMLnode& node, const Game* game) :
    Collision(node, game),
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

    m_newtonCollision = 
        NewtonCreateBox(game->m_world->m_world, m_size.x, m_size.y, m_size.z, 0);
}

void CollisionBox::render(const Video* video, const MaterialsMap* materials) const
{   
    glScalef(m_size.x, m_size.y, m_size.z);
    video->renderCube();
}

Collision* Collision::create(const XMLnode& node, const Game* game)
{
    string type = getAttribute(node, "type");
    if (type == "box")
    {
        return new CollisionBox(node, game);
    }
    else if (type == "tree")
    {
        return new CollisionTree(node, game);
    }
    else
    {
        throw Exception("Unknown collision type - " + type);
    }
}

CollisionTree::CollisionTree(const XMLnode& node, const Game* game) :
    Collision(node, game)
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
                    face.vertexes.push_back(getAttributesInVector(node, "xyz"));
                    face.uv.push_back(UV(
                        cast<string, float>(getAttribute(node, "u")),
                        cast<string, float>(getAttribute(node, "v"))));
                }
                else
                { 
                    throw Exception("Invalid face, unknown node - " + node.name);
                }
            }
            Vector v0 = m_faces.back().vertexes[0];
            Vector v1 = m_faces.back().vertexes[1];
            Vector v2 = m_faces.back().vertexes[2];
            
            face.normal = (v1-v0) ^ (v2-v0);
            face.normal.norm();
        }
        else
        {
            throw Exception("Invalid collision, unknown node - " + node.name);
        }
    }
    m_newtonCollision = 
        NewtonCreateBox(game->m_world->m_world, 0.1f, 0.1f, 0.1f, 0);
}

void CollisionTree::render(const Video* video, const MaterialsMap* materials) const
{
    for (size_t i = 0; i < m_faces.size(); i++)
    {
        materials->find(m_materials[i])->second->render(video);

        video->renderFace(m_faces[i]);
    }
}
