#include "collision.h"
#include "xml.h"
#include "video.h"
#include "material.h"
#include "level.h"
#include "property.h"
#include "properties.h"
#include "world.h"

class CollisionConvex : public Collision
{
protected:
    CollisionConvex(const XMLnode& node, const Level* level);

    Material* m_material;

    bool      m_hasOffset; // false
    Matrix    m_matrix;
    int       m_propertyID;
};

class CollisionBox : public CollisionConvex
{
public:
    CollisionBox(const XMLnode& node, const Level* level);
    void render() const;

    Vector m_size;      // (1.0f, 1.0f, 1.0f)
};

class CollisionSphere : public CollisionConvex
{
public:
    CollisionSphere(const XMLnode& node, const Level* level);
    void render() const;

    Vector m_radius;      // (1.0f, 1.0f, 1.0f)
};

class CollisionTree : public Collision
{
public:
    CollisionTree(const XMLnode& node, Level* level);
    ~CollisionTree();
    void render() const;

    vector<Face>         m_faces;
    vector<Material*>    m_materials;

    mutable bool         m_first;
    mutable unsigned int m_list;
};


Collision::Collision(const XMLnode& node) : m_inertia(), m_mass(0.0f), m_origin()
{
}

Collision::~Collision()
{
    //NewtonReleaseCollision(World::instance->m_newtonWorld, m_newtonCollision);
}

void Collision::create(NewtonCollision* collision)
{
    m_newtonCollision = collision;
}

void Collision::create(NewtonCollision* collision, int propertyID, float mass)
{
    m_newtonCollision = collision;
    m_mass = mass;

    NewtonConvexCollisionCalculateInertialMatrix(m_newtonCollision, m_inertia.v, m_origin.v);
    NewtonConvexCollisionSetUserID(m_newtonCollision, propertyID);

    m_inertia *= mass;
    m_origin *= mass;
}

Collision* Collision::create(const XMLnode& node, Level* level)
{
    string type = node.getAttribute("type");
    
    if (type == "box")
    {
        return new CollisionBox(node, level);
    }    
    if (type == "sphere")
    {
        return new CollisionSphere(node, level);
    }
    else if (type == "tree")
    {
        return new CollisionTree(node, level);
    }
    else
    {
        throw Exception("Unknown collision type - " + type);
    }
}

CollisionConvex::CollisionConvex(const XMLnode& node, const Level* level) :
    Collision(node),
    m_hasOffset(false),
    m_material(NULL)
{
    Vector offset(0.0f, 0.0f, 0.0f);
    Vector rotation(0.0f, 0.0f, 0.0f);

    if (node.hasAttribute("property"))
    {
        m_propertyID = level->m_properties->getPropertyID(node.getAttribute("property"));
    }
    else
    {
        m_propertyID = level->m_properties->getDefault();
    }

    if (node.hasAttribute("material"))
    {
        string material = node.getAttribute("material");
        MaterialMap::const_iterator iter = level->m_materials.find(material);
        if (iter == level->m_materials.end())
        {
            throw Exception("Couldn't find material '" + material + "'");
        }
        m_material = iter->second;;
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

CollisionBox::CollisionBox(const XMLnode& node, const Level* level) :
    CollisionConvex(node, level),
    m_size(1.0f, 1.0f, 1.0f)
{
    float mass = node.getAttribute<float>("mass");

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
            World::instance->m_newtonWorld, 
            m_size.x, m_size.y, m_size.z, 
            (m_hasOffset ? m_matrix.m : NULL)
        ),
        m_propertyID,
        mass);
}

void CollisionBox::render() const
{
    glPushMatrix();

    Video::instance->enableMaterial(m_material);

    if (m_hasOffset)
    {
        glMultMatrixf(m_matrix.m);
    }

    glScalef(m_size.x, m_size.y, m_size.z);
    Video::instance->renderCube();

    Video::instance->disableMaterial(m_material);

    glPopMatrix();
}

CollisionSphere::CollisionSphere(const XMLnode& node, const Level* level) :
    CollisionConvex(node, level),
    m_radius(1.0f, 1.0f, 1.0f)
{
    float mass = node.getAttribute<float>("mass");

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
            World::instance->m_newtonWorld, 
            m_radius.x, m_radius.y, m_radius.z, 
            (m_hasOffset ? m_matrix.m : NULL)
        ),
        m_propertyID,
        mass);
}

void CollisionSphere::render() const
{
    glPushMatrix();

    Video::instance->enableMaterial(m_material);

    if (m_hasOffset)
    {
        glMultMatrixf(m_matrix.m);
    }

    glScalef(m_radius.x, m_radius.y, m_radius.z);
    Video::instance->renderSphere();

    Video::instance->disableMaterial(m_material);

    glPopMatrix();
}

CollisionTree::CollisionTree(const XMLnode& node, Level* level) : 
    Collision(node), m_first(true), m_list(0)

{
    vector<int> props;

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "face")
        {
            string material = node.getAttribute("material");
            if (material.empty())
            {
                m_materials.push_back(NULL);
            }
            else
            {
                MaterialMap::const_iterator iter = level->m_materials.find(material);
                if (iter == level->m_materials.end())
                {
                    throw Exception("Couldn't find material '" + material + "'");
                }
                m_materials.push_back(iter->second);
            }
            
            string p = node.getAttribute("property");
            props.push_back(level->m_properties->getPropertyID(p));

            m_faces.push_back(Face());
            Face& face = m_faces.back();
            
            // needed in grass calculations

            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "vertex")
                {
                    face.vertexes.push_back(getAttributesInVector(node, "xyz"));
                    face.uv.push_back(UV(node.getAttribute<float>("u"), node.getAttribute<float>("v")));
                }
                else
                { 
                    throw Exception("Invalid face, unknown node - " + node.name);
                }
            }
            if (face.uv.size() != 4 && face.uv.size() != 3)
            {
                throw Exception("Face must have 3 or 4 vertexes");
            }

            const Vector& v0 = face.vertexes[0];
            const Vector& v1 = face.vertexes[1];
            const Vector& v2 = face.vertexes[2];
            
            Vector normal = (v1-v0) ^ (v2-v0);
            normal.norm();

            face.normal.resize(face.vertexes.size(), normal);

        }
        else
        {
            throw Exception("Invalid collision, unknown node - " + node.name);
        }
    }
    
    NewtonCollision* collision = NewtonCreateTreeCollision(World::instance->m_newtonWorld, NULL);
    NewtonTreeCollisionBeginBuild(collision);
    for (size_t i=0; i<m_faces.size(); i++)
    {
        int prop = props[i];
        Face& face = m_faces[i];
        NewtonTreeCollisionAddFace(
            collision, 
            static_cast<int>(face.vertexes.size()), 
            face.vertexes[0].v,
            sizeof(Vector),
            prop);
        
        std::swap(face.uv[2], face.uv[3]);
        std::swap(face.vertexes[2], face.vertexes[3]);
    }
    NewtonTreeCollisionEndBuild(collision, 0);
    
    create(collision);

    for (size_t i=0; i<m_faces.size(); i++)
    {
        level->m_faces.insert(make_pair(&m_faces[i], m_materials[i]));
    }
    /*
    m_buffers.resize(m_faces.size());
    Video::glGenBuffersARB(static_cast<GLsizei>(m_faces.size()), &m_buffers[0]);
    for (size_t i=0; i<m_faces.size(); i++)
    {
        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[i]);
        Video::glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(FaceBuf), &m_faces[i], GL_STATIC_DRAW_ARB);
    }
    */
}

void CollisionTree::render() const
{
    if (m_faces.size() == 0)
    {
        return;
    }

    if (m_first)
    {
        m_first = false;
        m_list = Video::instance->newList();
        glNewList(m_list, GL_COMPILE);

        const Material* last = m_materials.front();
        Video::instance->enableMaterial(last);
        for (size_t i = 0; i < m_faces.size(); i++)
        {
            if (last != m_materials[i] || i==m_faces.size()-1)
            {
                Video::instance->disableMaterial(last);
                Video::instance->enableMaterial(m_materials[i]);
                last = m_materials[i];
            }
            //this line does the same as whole if block above, but in more ineffective way
            //Video::instance->enableMaterial(m_materials[i]);

            //Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[i]);
            //glInterleavedArrays(GL_T2F_N3F_V3F, 0, NULL);
            //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
            Video::instance->renderFace(m_faces[i]);
        }
        Video::instance->disableMaterial(last);

        glEndList();
    }
    
    glCallList(m_list);
}

CollisionTree::~CollisionTree()
{
//    Video::glDeleteBuffersARB(static_cast<GLsizei>(m_faces.size()), &m_buffers[0]);
}
