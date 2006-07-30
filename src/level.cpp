#include "level.h"
#include "file.h"
#include "xml.h"
#include "video.h"
#include "game.h"
#include "world.h"

using namespace LevelObjects;

/////

    class CollisionConvex : public Collision
    {
    protected:
        CollisionConvex(const XMLnode& node, const NewtonWorld* newtonWorld, const MaterialsMap* materials);

        Material* m_material;
        bool   m_hasOffset; // false
        Matrix m_matrix;
    };

    class CollisionBox : public CollisionConvex
    {
    public:
        CollisionBox(const XMLnode& node, const NewtonWorld* newtonWorld, const MaterialsMap* materials);
        void render(const Video* video) const;

        Vector m_size;      // (1.0f, 1.0f, 1.0f)
    };

    class CollisionSphere : public CollisionConvex
    {
    public:
        CollisionSphere(const XMLnode& node, const NewtonWorld* newtonWorld, const MaterialsMap* materials);
        void render(const Video* video) const;

        Vector m_radius;      // (1.0f, 1.0f, 1.0f)
    };

    class CollisionTree : public Collision
    {
    public:
        CollisionTree(const XMLnode& node, const NewtonWorld* newtonWorld, const MaterialsMap* materials);
        void render(const Video* video) const;

        vector<Face>      m_faces;
        vector<Material*> m_materials;
    };

/////

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
        vector[i] = cast<float>(getAttribute(node, key));
    }
    return vector;

}

Material::Material(const XMLnode& node, Video* video) :
    m_id(),
    m_shader(NULL),
    m_cAmbient(0.2f, 0.2f, 0.2f),
    m_cSpecular(0.0f, 0.0f, 0.0f),
    m_cEmission(0.0f, 0.0f, 0.0f),
    m_cShine(0.0f),
    m_texture(0),
    m_textureBump(0)
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
                    m_texture = video->loadTexture(node.value);
                }
                else if (node.name == "bump_path")
                {
                    m_textureBump = video->loadTexture(node.value);
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
                    m_cShine = cast<float>(node.value);
                }
                else
                {
                    throw Exception("Invalid color, unknown node - " + node.name);
                }
            }
        }
        else if (node.name == "shader")
        {
            std::string name = getAttribute(node, "name");
            m_shader = video->loadShader(name + ".vp", name + ".fp");
        }
        else
        {
            throw Exception("Invalid material, unknown node - " + node.name);
        }
    }
}

void Material::enable(const Video* video) const
{
    Video::glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, m_cAmbient.v);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_cSpecular.v);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, m_cEmission.v);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_cShine);

    if (video->m_haveShaders && (m_shader != NULL))
    {
        Video::glActiveTextureARB(GL_TEXTURE1_ARB);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_textureBump);
        video->begin(m_shader);
    }
}

void Material::disable(const Video* video) const
{
    if (video->m_haveShaders && (m_shader != NULL))
    {
        video->end(m_shader);
        Video::glActiveTextureARB(GL_TEXTURE1_ARB);
        glDisable(GL_TEXTURE_2D);
    }

    Video::glActiveTextureARB(GL_TEXTURE0_ARB);
    glDisable(GL_TEXTURE_2D);
}

Level::Level(const Game* game) : m_game(game)
{
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
                    std::string id = getAttribute(node, "id");
                    m_bodies[id] = new Body(node, m_game);
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
                    m_materials[getAttribute(node, "id")] = new Material(node, m_game->m_video.get());
                }
                else if (node.name == "properties")
                {
                    // TODO: load NewtonMaterial properties
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
                    m_collisions[getAttribute(node, "id")] = Collision::create(node, m_game->m_world->m_newtonWorld, &m_materials);
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
    for each_(MaterialsMap, m_materials, iter)
    {
        delete iter->second;
    }
}

void Level::prepare()
{
    for each_const(BodiesMap, m_bodies, iter)
    {
        (iter->second)->prepare();
    }
}

void Level::render(const Video* video) const
{
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_COLOR_MATERIAL);
    for each_const(BodiesMap, m_bodies, iter)
    {
       (iter->second)->render(video);
    }    
    glPopAttrib();
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


Body::Body(const XMLnode& node, const Game* game):
    m_matrix(),
    m_totalMass(0.0f),
    m_totalInertia(0.0f, 0.0f, 0.0f),
    m_newtonWorld(game->m_world->m_newtonWorld)
{
    NewtonCollision* newtonCollision = NULL;
    string id = getAttribute(node, "id");

    Vector position(0.0f, 0.0f, 0.0f);
    Vector rotation(0.0f, 0.0f, 0.0f);

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "position")
        {
            position = getAttributesInVector(node, "xyz");
        }
        else if (node.name == "rotation")
        {
            rotation = getAttributesInVector(node, "xyz") * DEG_IN_RAD;
        }
        else if (node.name == "collision")
        { 
            string id = node.value;
            CollisionsMap::const_iterator iter = game->m_world->m_level->m_collisions.find(id);
            if (iter != game->m_world->m_level->m_collisions.end())
            {
                m_collisions.insert(iter->second);
            }
            else
            {
                throw Exception("Could not find specified collision for body '" + id + "'");
            }
        }
        else
        {
            throw Exception("Invalid body, unknown node - " + node.name);
        }
    }

    Vector totalOrigin(0.0f, 0.0f, 0.0f);

    if (m_collisions.size() == 1)
    {
        Collision* collision = *m_collisions.begin();

        m_totalMass = collision->m_mass;
        m_totalInertia = collision->m_inertia;
        totalOrigin = collision->m_origin;

        newtonCollision = collision->m_newtonCollision;
    }
    else if (m_collisions.size() > 1)
    {
        vector<NewtonCollision*> newtonCollisions(m_collisions.size());
        int cnt = 0;
        for each_const(set<Collision*>, m_collisions, iter)
        {
            Collision* collision = *iter;
            newtonCollisions[cnt++] = collision->m_newtonCollision;
            m_totalMass += collision->m_mass;
            m_totalInertia += collision->m_inertia;
            totalOrigin += collision->m_origin;
        }
        totalOrigin /= m_totalMass;

        newtonCollision = NewtonCreateCompoundCollision(
                                                m_newtonWorld,
                                                cnt,
                                                &newtonCollisions[0]);
        for each_const(vector<NewtonCollision*>, newtonCollisions, collision)
        {
            NewtonReleaseCollision(m_newtonWorld, *collision);
        }
    }
    else
    {
        throw Exception("No collisions were found for body '" + id + "'");
    }

    createNewtonBody(newtonCollision, totalOrigin, position, rotation);

    if (id == "football")
    {
        NewtonBodySetContinuousCollisionMode(m_newtonBody, 1);
    }
}

void Body::setPositionAndRotation(const Vector& position,
                                  const Vector& rotation)
{
    NewtonSetEulerAngle(rotation.v, m_matrix.m);
    m_matrix = Matrix::translate(position) * m_matrix;
            
    NewtonBodySetMatrix(m_newtonBody, m_matrix.m);
}

void Body::createNewtonBody(const NewtonCollision* newtonCollision,
                            const Vector&          totalOrigin,
                            const Vector&          position,
                            const Vector&          rotation)
{
    m_newtonBody = NewtonCreateBody(m_newtonWorld, newtonCollision);
    NewtonBodySetUserData(m_newtonBody, static_cast<void*>(this));

    // Set Material Id for this object
    //NewtonBodySetMaterialGroupID(m_body, material);

    NewtonBodySetMassMatrix(m_newtonBody, m_totalMass, m_totalInertia.x, m_totalInertia.y, m_totalInertia.z);
    NewtonBodySetCentreOfMass(m_newtonBody, totalOrigin.v);
    
    NewtonBodySetForceAndTorqueCallback(m_newtonBody, onSetForceAndTorque);

    setPositionAndRotation(position, rotation);

    NewtonBodySetAutoFreeze(m_newtonBody, 0);

    NewtonReleaseCollision(m_newtonWorld, newtonCollision);
}

Body::~Body()
{
    for each_const(set<Collision*>, m_collisions, iter)
    {
        //NewtonReleaseCollision(m_newtonWorld, (*iter)->m_newtonCollision);
    }
    //for each_const(set<Body*>, m_clones, iter)
    {
        //delete *iter;
        
    }


}

void Body::prepare()
{
    NewtonBodyGetMatrix(m_newtonBody, m_matrix.m);
}

Vector Body::getPosition()
{
    return m_matrix.row(3);
}

void Body::onSetForceAndTorque()
{
    Vector force = gravityVec * m_totalMass;
    NewtonBodyAddForce(m_newtonBody, force.v);
}

void Body::onSetForceAndTorque(const NewtonBody* body)
{
    Body* self = static_cast<Body*>(NewtonBodyGetUserData(body));
    self->onSetForceAndTorque();
}

void Body::render(const Video* video)
{
    video->begin(m_matrix);

    for each_const(set<Collision*>, m_collisions, iter)
    {
        (*iter)->render(video);
    }

    video->end();
}

Collision::Collision(const XMLnode& node, const NewtonWorld* newtonWorld) :
    m_inertia(), m_mass(0.0f), m_origin(), m_newtonWorld(newtonWorld)
{
}

Collision::~Collision()
{
    //NewtonReleaseCollision(m_newtonWorld, m_newtonCollision);
}

void Collision::create(NewtonCollision* collision)
{
    m_newtonCollision = collision;
}

void Collision::create(NewtonCollision* collision, float mass)
{
    m_newtonCollision = collision;
    m_mass = mass;

    NewtonConvexCollisionCalculateInertialMatrix(m_newtonCollision, m_inertia.v, m_origin.v);
    m_inertia *= mass;
    m_origin *= mass;
}

Collision* Collision::create(const XMLnode& node, const NewtonWorld* newtonWorld, const MaterialsMap* materials)
{
    string type = getAttribute(node, "type");
    
    if (type == "box")
    {
        return new CollisionBox(node, newtonWorld, materials);
    }    
    if (type == "sphere")
    {
        return new CollisionSphere(node, newtonWorld, materials);
    }
    else if (type == "tree")
    {
        return new CollisionTree(node, newtonWorld, materials);
    }
    else
    {
        throw Exception("Unknown collision type - " + type);
    }
}

CollisionConvex::CollisionConvex(const XMLnode& node, const NewtonWorld* newtonWorld, const MaterialsMap* materials) :
    Collision(node, newtonWorld),
    m_hasOffset(false),
    m_material(NULL)
{
    Vector offset(0.0f, 0.0f, 0.0f);
    Vector rotation(0.0f, 0.0f, 0.0f);

    string name = "material";
    if (foundInMap(node.attributes, name))
    {
        MaterialsMap::const_iterator iter = materials->find(getAttribute(node, name));
        if (iter != materials->end())
        {
            m_material = iter->second;
        }
    }

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "offset")
        {
            offset = getAttributesInVector(node, "xyz");
            m_hasOffset = true;
        }
        else if (node.name == "rotation")
        {
            rotation = getAttributesInVector(node, "xyz") * DEG_IN_RAD;
            m_hasOffset = true;
        }
    }

    if (m_hasOffset)
    {
        NewtonSetEulerAngle(rotation.v, m_matrix.m);
        m_matrix = Matrix::translate(offset) * m_matrix;
    }
}

CollisionBox::CollisionBox(const XMLnode& node, const NewtonWorld* newtonWorld, const MaterialsMap* materials) :
    CollisionConvex(node, newtonWorld, materials),
    m_size(1.0f, 1.0f, 1.0f)
{
    float mass = cast<float>(getAttribute(node, "mass"));

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "size")
        {
            m_size = getAttributesInVector(node, "xyz");
        }
        else if (node.name != "offset" && node.name != "rotation")
        {
            throw Exception("Invalid collision, unknown node - " + node.name);
        }
    }

    create(
        NewtonCreateBox(
            newtonWorld, 
            m_size.x, m_size.y, m_size.z, 
            (m_hasOffset ? m_matrix.m : NULL)
        ),
        mass);
}

void CollisionBox::render(const Video* video) const
{
    glPushMatrix();

    video->enableMaterial(m_material);

    if (m_hasOffset)
    {
        glMultMatrixf(m_matrix.m);
    }

    glScalef(m_size.x, m_size.y, m_size.z);
    video->renderCube();

    video->disableMaterial(m_material);

    glPopMatrix();
}

CollisionSphere::CollisionSphere(const XMLnode& node, const NewtonWorld* newtonWorld, const MaterialsMap* materials) :
    CollisionConvex(node, newtonWorld, materials),
    m_radius(1.0f, 1.0f, 1.0f)
{
    float mass = cast<float>(getAttribute(node, "mass"));

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "radius")
        {
            m_radius = getAttributesInVector(node, "xyz");
        }
        else if (node.name != "offset" && node.name != "rotation")
        {
            throw Exception("Invalid collision, unknown node - " + node.name);
        }
    }

    create(
        NewtonCreateSphere(
            newtonWorld, 
            m_radius.x, m_radius.y, m_radius.z, 
            (m_hasOffset ? m_matrix.m : NULL)
        ),
        mass);
}

void CollisionSphere::render(const Video* video) const
{
    glPushMatrix();

    video->enableMaterial(m_material);

    if (m_hasOffset)
    {
        glMultMatrixf(m_matrix.m);
    }

    glScalef(m_radius.x, m_radius.y, m_radius.z);
    video->renderSphere();

    video->disableMaterial(m_material);

    glPopMatrix();
}

CollisionTree::CollisionTree(const XMLnode& node, const NewtonWorld* newtonWorld, const MaterialsMap* materials) :
    Collision(node, newtonWorld)
{
    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "face")
        {
            MaterialsMap::const_iterator iter = materials->find(getAttribute(node, "material"));
            if (iter != materials->end())
            {
                m_materials.push_back(iter->second);
            }
            else
            {
                m_materials.push_back(NULL);
            }
 
            m_faces.push_back(Face());
            Face& face = m_faces.back();
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "vertex")
                {
                    face.vertexes.push_back(getAttributesInVector(node, "xyz"));
                    face.uv.push_back(UV(
                        cast<float>(getAttribute(node, "u")),
                        cast<float>(getAttribute(node, "v"))));
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
    NewtonCollision* collision = NewtonCreateTreeCollision(newtonWorld, NULL);
    NewtonTreeCollisionBeginBuild(collision);
    for each_const(vector<Face>, m_faces, iter)
    {
        NewtonTreeCollisionAddFace(collision, static_cast<int>(iter->vertexes.size()), iter->vertexes[0].v, sizeof(Vector), 0);
    }
    NewtonTreeCollisionEndBuild(collision, 0);
    
    create(collision);
}

void CollisionTree::render(const Video* video) const
{
    if (m_faces.size() == 0)
    {
        return;
    }

    const Material* last = m_materials[0];
    video->enableMaterial(last);
    for (size_t i = 0; i < m_faces.size(); i++)
    {
        if (last != m_materials[i])
        {
            video->disableMaterial(last);
            video->enableMaterial(m_materials[i]);
            last = m_materials[i];
        }
        video->renderFace(m_faces[i]);
    }
    video->disableMaterial(last);
/*
    for (size_t i = 0; i < m_faces.size(); i++)
    {
        video->enableMaterial(m_materials[i]);
        video->renderFace(m_faces[i]);
        video->disableMaterial(m_materials[i]);
    }
*/
}

