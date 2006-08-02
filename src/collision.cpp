#include "collision.h"
#include "xml.h"
#include "video.h"
#include "material.h"
#include "level.h"
#include "materials.h"

class CollisionConvex : public Collision
{
protected:
    CollisionConvex(const XMLnode& node, 
                    const NewtonWorld* newtonWorld, 
                    const Level* level);

    Material* m_material;
    bool      m_hasOffset; // false
    Matrix    m_matrix;
};

class CollisionBox : public CollisionConvex
{
public:
    CollisionBox(const XMLnode& node, 
                 const NewtonWorld* newtonWorld, 
                 const Level* level);
    void render(const Video* video) const;

    Vector m_size;      // (1.0f, 1.0f, 1.0f)
};

class CollisionSphere : public CollisionConvex
{
public:
    CollisionSphere(const XMLnode& node, const 
                    NewtonWorld* newtonWorld, 
                    const Level* level);
    void render(const Video* video) const;

    Vector m_radius;      // (1.0f, 1.0f, 1.0f)
};

class CollisionTree : public Collision
{
public:
    CollisionTree(const XMLnode& node, 
                  const NewtonWorld* newtonWorld, 
                  const Level* level);
    void render(const Video* video) const;

    vector<Face>      m_faces;
    vector<Material*> m_materials;
};



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

Collision* Collision::create(const XMLnode& node, 
                             const NewtonWorld* newtonWorld, 
                             const Level* level)
{
    string type = getAttribute(node, "type");
    
    if (type == "box")
    {
        return new CollisionBox(node, newtonWorld, level);
    }    
    if (type == "sphere")
    {
        return new CollisionSphere(node, newtonWorld, level);
    }
    else if (type == "tree")
    {
        return new CollisionTree(node, newtonWorld, level);
    }
    else
    {
        throw Exception("Unknown collision type - " + type);
    }
}

CollisionConvex::CollisionConvex(const XMLnode& node, 
                                 const NewtonWorld* newtonWorld, 
                                 const Level* level) :
    Collision(node, newtonWorld),
    m_hasOffset(false),
    m_material(NULL)
{
    Vector offset(0.0f, 0.0f, 0.0f);
    Vector rotation(0.0f, 0.0f, 0.0f);

    string name = "material";
    if (foundInMap(node.attributes, name))
    {
        m_material = level->m_materials->get(getAttribute(node, name));
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

CollisionBox::CollisionBox(const XMLnode& node, const NewtonWorld* newtonWorld, const Level* level) :
    CollisionConvex(node, newtonWorld, level),
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

CollisionSphere::CollisionSphere(const XMLnode& node, const NewtonWorld* newtonWorld, const Level* level) :
    CollisionConvex(node, newtonWorld, level),
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

CollisionTree::CollisionTree(const XMLnode& node, const NewtonWorld* newtonWorld, const Level* level) :
    Collision(node, newtonWorld)
{
    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "face")
        {
            string material = getAttribute(node, "material");
            if (material.empty())
            {
                m_materials.push_back(NULL);
            }
            else
            {
                m_materials.push_back(level->m_materials->get(material));
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
    for (size_t i=0; i<m_faces.size(); i++)
    {
        Material* material = m_materials[i];
        Face& face = m_faces[i];
        NewtonTreeCollisionAddFace(
            collision, 
            static_cast<int>(face.vertexes.size()), 
            face.vertexes[0].v, 
            sizeof(Vector), *reinterpret_cast<int*>(&material));
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

