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

class CollisionHMap : public Collision
{
public:
    CollisionHMap(const XMLnode& node, Level* level);
    ~CollisionHMap();
    
    void render() const;

    vector<Vector>         m_vertices;
    vector<Vector>         m_normals;
    vector<unsigned short> m_indices;

    unsigned int m_buffers[3];
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
    else if (type == "heightmap")
    {
        return new CollisionHMap(node, level);
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

CollisionHMap::CollisionHMap(const XMLnode& node, Level* level) : Collision(node)
{
    string hmap;
    float size;

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "heightmap")
        {
            hmap = node.getAttribute("name");
            size = node.getAttribute<float>("size");
        }
        else
        {
            throw Exception("Invalid collision, unknown node - " + node.name);
        }
    }
    
    if (hmap.empty())
    {
        throw Exception("Invalid heightmap collision, heightmap name not specified");
    }

    string filename = "/data/heightmaps/" + hmap + ".tga";
    File::Reader file(filename);
    if (!file.is_open())
    {
        throw Exception("Heightmap '" + filename + "' not found");
    }

    vector<char> data(file.size());
    file.read(&data[0], data.size());
    file.close();
    
    GLFWimage image;
    if (glfwReadMemoryImage(&data[0], static_cast<int>(data.size()), &image, GLFW_NO_RESCALE_BIT)==GL_FALSE)
    {
        throw Exception("Invalid heightmap '" + filename + "' format");
    }

    if (image.BytesPerPixel != 1)
    {
        throw Exception("Invalid heightmap '" + filename + "', image must be grayscale");
    }

    int id = level->m_properties->getDefault();

    NewtonCollision* collision = NewtonCreateTreeCollision(World::instance->m_newtonWorld, NULL);
    NewtonTreeCollisionBeginBuild(collision);


    float size2 = size/2.0f;
    float x = -size2;
    float STEP = 0.4f;

    int maxIdx = 0;
    bool maxIdxB = false;

    while (x < size2)
    {
        int ix = static_cast<int>((x + size2) * image.Width / size);
        int ix2 = static_cast<int>((x + size2 + STEP) * image.Width / size);
        if (ix2 >= image.Width) break;

        float z = -size2;
        while (z < size2)
        {
            int iz = static_cast<int>((z + size2) * image.Height / size);
            int iz2 = static_cast<int>((z + size2 + STEP) * image.Height / size);
            if (iz2 >= image.Height) break;
            
            float y1 = (image.Data[image.Width * iz + ix] - 128) / 10.0f;
            float y2 = (image.Data[image.Width * iz2 + ix] - 128) / 10.0f;
            float y3 = (image.Data[image.Width * iz2 + ix2] - 128) / 10.0f;
            float y4 = (image.Data[image.Width * iz + ix2] - 128) / 10.0f;

            const Vector v0 = Vector(x, y1, z);
            const Vector v1 = Vector(x, y2, z+STEP);
            const Vector v2 = Vector(x+STEP, y3, z+STEP);
            const Vector v3 = Vector(x+STEP, y3, z);
            
            Vector normal = (v1 - v0)  ^ (v3 - v0);
            normal.norm();
                
            m_vertices.push_back(v0);
            m_normals.push_back(normal);

            const Vector arr1[] = { v0, v1, v2 };
            const Vector arr2[] = { v1, v2, v3 };
            NewtonTreeCollisionAddFace(collision, 3, arr1[0].v, sizeof(Vector), id);
            NewtonTreeCollisionAddFace(collision, 3, arr2[0].v, sizeof(Vector), id);

            unsigned int idx = static_cast<unsigned int>(m_vertices.size());
 
            z += STEP;
            if (!maxIdxB) maxIdx++;
        }
        if (!maxIdxB) maxIdxB = true;
        x += STEP;
    }

    for (int x=0; x<maxIdx-1; x++)
    {
        for (int z=0; z<maxIdx-1; z++)
        {
            int i1 = x*maxIdx+z;
            int i2 = x*maxIdx+z+1;
            int i3 = (x+1)*maxIdx+z+1;
            int i4 = (x+1)*maxIdx+z;
            
            if (i3 > 65535)
            {
                throw Exception("Too many vertieces in heightmap!!");
            }

            m_indices.push_back(i1);
            m_indices.push_back(i2);
            m_indices.push_back(i4);

            m_indices.push_back(i2);
            m_indices.push_back(i3);
            m_indices.push_back(i4);
        }
    }
    
    NewtonTreeCollisionEndBuild(collision, 0);
    
    create(collision);

    glfwFreeImage(&image);
    
    if (Video::instance->m_haveVBO)
    {
        Video::glGenBuffersARB(3, &m_buffers[0]);

        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[0]);
        Video::glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_vertices.size() * sizeof(Vector), &m_vertices[0], GL_STATIC_DRAW_ARB);

        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[1]);
        Video::glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_normals.size() * sizeof(Vector), &m_normals[0], GL_STATIC_DRAW_ARB);

        Video::glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_buffers[2]);
        Video::glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_indices.size() * sizeof(unsigned short), &m_indices[0], GL_STATIC_DRAW_ARB);

        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        Video::glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
}

void CollisionHMap::render() const
{
    if (m_indices.size() == 0)
    {
        return;
    }

    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    if (Video::instance->m_haveVBO)
    {
        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[0]);
        glNormalPointer(GL_FLOAT, sizeof(Vector), NULL);

        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[1]);
        glVertexPointer(4, GL_FLOAT, 0, NULL);

        Video::glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_buffers[2]);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_SHORT, NULL);

        Video::glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
    else
    {
        glNormalPointer(GL_FLOAT, sizeof(Vector), &m_normals[0]);
        glVertexPointer(3, GL_FLOAT, sizeof(Vector), &m_vertices[0]);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_SHORT, &m_indices[0]);
    }

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
}

CollisionHMap::~CollisionHMap()
{
    if (Video::instance->m_haveVBO)
    {
        Video::glDeleteBuffersARB(3, &m_buffers[0]);
    }
}
