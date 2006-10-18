#include "collision.h"
#include "xml.h"
#include "video.h"
#include "material.h"
#include "level.h"
#include "property.h"
#include "properties.h"
#include "world.h"
#include "texture.h"
#include "config.h"
#include "geometry.h"
#include "input.h"

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

    float getRadius() const { return m_radius.x; }

    Vector m_radius;      // (1.0f, 1.0f, 1.0f)
};

class CollisionCylinder : public CollisionConvex
{
public:
    CollisionCylinder(const XMLnode& node, const Level* level);
    void render() const;

    float m_radius;      // 1.0f
    float m_height;      // 1.0f
};

class CollisionCone : public CollisionConvex
{
public:
    CollisionCone(const XMLnode& node, const Level* level);
    void render() const;

    float m_radius;      // 1.0f
    float m_height;      // 1.0f
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
    void renderTri(float x, float z) const;
    float getHeight(float x, float y) const;

private:
    int                    m_width;
    int                    m_height;
    float                  m_size;

    vector<UV>             m_uv;
    vector<Vector>         m_normals;
    vector<Vector>         m_vertices;
    vector<unsigned short> m_indices;
    vector<unsigned short> m_indices2;
    
    int                    m_realCount;

    Material*    m_material;
    unsigned int m_buffers[5];

    vector<Face> m_tmpFaces;
};

Collision::Collision(const XMLnode& node) :
    m_origin(),
    m_inertia(),
    m_mass(0.0f)
{
}

Collision::~Collision()
{
    //NewtonReleaseCollision(World::instance->m_newtonWorld, m_newtonCollision);
}

float Collision::getHeight(float x, float z) const
{
    return 0;
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
    else if (type == "sphere")
    {
        return new CollisionSphere(node, level);
    }
    else if (type == "cylinder")
    {
        return new CollisionCylinder(node, level);
    }
    else if (type == "cone")
    {
        return new CollisionCone(node, level);
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
        MaterialsMap::const_iterator iter = level->m_materials.find(material);
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
            offset = node.getAttributesInVector("xyz");
            m_hasOffset = true;
        }
        else if (node.name == "rotation")
        {
            rotation = node.getAttributesInVector("xyz") * DEG_IN_RAD;
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
            m_size = node.getAttributesInVector("xyz");
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

    Video::instance->bind(m_material);

    if (m_hasOffset)
    {
        glMultMatrixf(m_matrix.m);
    }

    glEnable(GL_NORMALIZE);
    glScalef(m_size.x, m_size.y, m_size.z);
    Video::instance->renderCube();
    glDisable(GL_NORMALIZE);

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
            m_radius = node.getAttributesInVector("xyz");
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

    Video::instance->bind(m_material);

    if (m_hasOffset)
    {
        glMultMatrixf(m_matrix.m);
    }

    glEnable(GL_NORMALIZE);
    glScalef(m_radius.x, m_radius.y, m_radius.z);
    Video::instance->renderSphere();
    glDisable(GL_NORMALIZE);

    glPopMatrix();
}

CollisionCylinder::CollisionCylinder(const XMLnode& node, const Level* level) :
    CollisionConvex(node, level),
    m_radius(1.0f),
    m_height(1.0f)
{
    float mass = node.getAttribute<float>("mass");

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "radius")
        {
            m_radius = cast<float>(node.value);
        }
        else if (node.name == "height")
        {
            m_height = cast<float>(node.value);
        }
        else if (node.name != "offset" && node.name != "rotation")
        {
            throw Exception("Invalid collision, unknown node - " + node.name);
        }
    }

    create(
        NewtonCreateCylinder(
            World::instance->m_newtonWorld, 
            m_radius,
            m_height,
            (m_hasOffset ? m_matrix.m : NULL)
        ),
        m_propertyID,
        mass);
}

void CollisionCylinder::render() const
{
    glPushMatrix();

    Video::instance->bind(m_material);

    if (m_hasOffset)
    {
        glMultMatrixf(m_matrix.m);
    }
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, -m_height/2.0f);
    Video::instance->renderCylinder(m_radius, m_height);

    glPopMatrix();
}

CollisionCone::CollisionCone(const XMLnode& node, const Level* level) :
    CollisionConvex(node, level),
    m_radius(1.0f),
    m_height(1.0f)
{
    float mass = node.getAttribute<float>("mass");

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "radius")
        {
            m_radius = cast<float>(node.value);
        }
        else if (node.name == "height")
        {
            m_height = cast<float>(node.value);
        }
        else if (node.name != "offset" && node.name != "rotation")
        {
            throw Exception("Invalid collision, unknown node - " + node.name);
        }
    }

    create(
        NewtonCreateCone(
            World::instance->m_newtonWorld, 
            m_radius,
            m_height,
            (m_hasOffset ? m_matrix.m : NULL)
        ),
        m_propertyID,
        mass);
}

void CollisionCone::render() const
{
    glPushMatrix();

    Video::instance->bind(m_material);

    if (m_hasOffset)
    {
        glMultMatrixf(m_matrix.m);
    }
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, -m_height/2.0f);
    Video::instance->renderCone(m_radius, m_height);

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
                MaterialsMap::const_iterator iter = level->m_materials.find(material);
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
                    face.vertexes.push_back(node.getAttributesInVector("xyz"));
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
        //level->m_faces.insert(make_pair(&m_faces[i], m_materials[i]));
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

        for (size_t i = 0; i < m_faces.size(); i++)
        {
            Video::instance->bind(m_materials[i]);

            Video::instance->renderFace(m_faces[i]);
            //Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[i]);
            //glInterleavedArrays(GL_T2F_N3F_V3F, 0, NULL);
            //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
        }

        glEndList();
    }
    
    glCallList(m_list);
}

CollisionTree::~CollisionTree()
{
//    Video::glDeleteBuffersARB(static_cast<GLsizei>(m_faces.size()), &m_buffers[0]);
}

CollisionHMap::CollisionHMap(const XMLnode& node, Level* level) : Collision(node), m_material(NULL)
{
    string hmap;
    float size = 0.0f;
    string material;
    float repeat = 0.0f;

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "heightmap")
        {
            hmap = node.getAttribute("name");
            size = node.getAttribute<float>("size");
            material = node.getAttribute("material");
            repeat = node.getAttribute<float>("repeat");
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
    if (material.empty())
    {
        throw Exception("Invalid heightmap collision, material name not specified");
    }
    if (repeat == 0.0f)
    {
        throw Exception("Invalid heightmap collision, repeat not specified");
    }

    m_material = level->m_materials.find(material)->second;
    const float c = repeat/size; //1.5f; //m_texture->m_size;//1.5f;

    int id = level->m_properties->getPropertyID("grass");

    NewtonCollision* collision = NewtonCreateTreeCollision(World::instance->m_newtonWorld, NULL);
    NewtonTreeCollisionBeginBuild(collision);
     
#if 0    
    if (File::exists("/data/heightmaps/" + hmap + ".hmap"))
    {
        string filename = "/data/heightmaps/" + hmap + ".hmap";
        File::Reader file(filename);
        if (!file.is_open())
        {
            throw Exception("Heightmap '" + filename + "' not found");
        }
        char head[4];
        file.read(head, 4);
        if (string(head, head+4) != "HMAP")
        {
            throw Exception("Invalid heightmap '" + filename + "' format");
        }
        int vlen, ilen;
        file.read(&vlen, sizeof(vlen));
        file.read(&ilen, sizeof(ilen));

        m_vertices.resize(vlen);
        m_normals.resize(vlen);
        m_uv.resize(vlen);

        m_indices.resize(3*ilen);

        file.read(&m_vertices[0], sizeof(Vector)*vlen);
        file.read(&m_normals[0], sizeof(Vector)*vlen);
        file.read(&m_uv[0], sizeof(UV)*vlen);
        file.read(&m_indices[0], sizeof(unsigned short)*3*ilen);
        
        file.close();

        m_tmpFaces.resize(3*ilen);
        for (int i=0; i<ilen*3; i+=3)
        {
            int i0 = m_indices[i+0];
            int i1 = m_indices[i+1];
            int i2 = m_indices[i+2];

            const Vector arr[] = { m_vertices[i0], m_vertices[i1], m_vertices[i2] };
            NewtonTreeCollisionAddFace(collision, 3, arr[0].v, sizeof(Vector), id);

            Face* face = & m_tmpFaces[i];
            face->vertexes.resize(3);
            face->vertexes[0] = m_vertices[i0];
            face->vertexes[1] = m_vertices[i1];
            face->vertexes[2] = m_vertices[i2];
            level->m_faces.insert(make_pair(face, level->m_materials["grass"]));
        }
    }
    else
#endif
    {
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

        m_size = size;
        m_width = image.Width;
        m_height = image.Height;
        
        vector<unsigned char> m_hmap; // TODO: rename
        m_hmap.assign(image.Data, image.Data + m_width * m_height);

        float size2 = size/2.0f;

        // 0.5f, 1.0f, 1.5f
        float terrain_detail = static_cast<float>(Config::instance->m_video.terrain_detail);
        float STEP = ((2-terrain_detail)+1)/2.0f;

        int maxIdx = 0;
        bool maxIdxB = false;

        float z = -size2;
        while (true)
        {
            bool badMargin = false; // for normal

            float z2 = z + STEP;
            int iz = static_cast<int>(std::floor((z + size2) * image.Height / size));
            int iz2 = static_cast<int>(std::floor((z2 + size2) * image.Height / size));
            if (iz >= image.Height)
            {
                z = size2;
                iz = image.Height-1;
            }
            if (iz2 >= image.Height)
            {
                z2 = size2;
                iz2 = image.Height-1;
                badMargin = true;
            }

            float x = -size2;
            while (true)
            {
                float x2 = x + STEP;
                int ix = static_cast<int>(std::floor((x + size2) * image.Width / size));
                int ix2 = static_cast<int>(std::floor((x2 + size2) * image.Width / size));
                if (ix >= image.Width)
                {
                    x = size2;
                    ix = image.Width-1;
                }
                if (ix2 >= image.Width)
                {
                    x2 = size2;
                    ix2 = image.Width-1;
                    badMargin = true;
                }

                float y1 = (image.Data[image.Width * iz + ix] - 128) / 20.0f;
                float y2 = (image.Data[image.Width * iz2 + ix] - 128) / 20.0f;
                float y4 = (image.Data[image.Width * iz + ix2] - 128) / 20.0f;

                const Vector v0 = Vector(x, y1, z);
                const Vector v1 = Vector(x, y2, z2);
                const Vector v3 = Vector(x2, y4, z);
                
                Vector normal;
                if (badMargin)
                {
                    normal = m_normals.back();
                }
                else
                {
                    normal = (v1 - v0)  ^ (v3 - v0);
                    normal.norm();
                }

                m_vertices.push_back(v0);
                m_normals.push_back(normal);
                m_uv.push_back(UV((x+size2)*c, (z+size2)*c));

                if (x == size2)
                {
                    break;
                }
                x += STEP;
            }
            if (!maxIdxB)
            {
                maxIdx = static_cast<int>(m_vertices.size());
                maxIdxB = true;
            }

            if (z == size2)
            {
                break;
            }
            z += STEP;
        }
      
        glfwFreeImage(&image);

        m_realCount = maxIdx;

        if (m_material->m_id == "grass")
        {
            m_tmpFaces.resize(2*maxIdx*maxIdx);
        }

        int cnt = 0;

        for (int z=0; z<maxIdx-1; z++)
        {
            for (int x=0; x<maxIdx-1; x++)
            {
                int i1 = z*maxIdx+x;
                int i2 = (z+1)*maxIdx+x;
                int i3 = (z+1)*maxIdx+x+1;
                int i4 = z*maxIdx+x+1;

                const Vector& v1 = m_vertices[i1];
                const Vector& v2 = m_vertices[i2];
                const Vector& v3 = m_vertices[i3];
                const Vector& v4 = m_vertices[i4];
                
                if (i3 > 65535)
                {
                    throw Exception("Too many vertices in heightmap!!");
                }

                if (isPointInRectangle(v1, g_fieldLower, g_fieldUpper) &&
                    isPointInRectangle(v2, g_fieldLower, g_fieldUpper) &&
                    isPointInRectangle(v4, g_fieldLower, g_fieldUpper))
                {
                    m_indices.push_back(i1);
                    m_indices.push_back(i2);
                    m_indices.push_back(i4);
                }
                else
                {
                    m_indices2.push_back(i1);
                    m_indices2.push_back(i2);
                    m_indices2.push_back(i4);
                }

                if (isPointInRectangle(v2, g_fieldLower, g_fieldUpper) &&
                    isPointInRectangle(v3, g_fieldLower, g_fieldUpper) &&
                    isPointInRectangle(v4, g_fieldLower, g_fieldUpper))
                {
                    m_indices.push_back(i2);
                    m_indices.push_back(i3);
                    m_indices.push_back(i4);
                }
                else
                {
                    m_indices2.push_back(i2);
                    m_indices2.push_back(i3);
                    m_indices2.push_back(i4);
                }

                {
                    const Vector arr[] = { v1, v2, v4 };
                    NewtonTreeCollisionAddFace(collision, 3, arr[0].v, sizeof(Vector), id);
                }
                {
                    const Vector arr[] = { v2, v3, v4 };
                    NewtonTreeCollisionAddFace(collision, 3, arr[0].v, sizeof(Vector), id);
                }

                if (m_material->m_id == "grass")
                {
                    Face* face = & m_tmpFaces[cnt++];
                    face->vertexes.resize(3);
                    face->vertexes[0] = v1;
                    face->vertexes[1] = v2;
                    face->vertexes[2] = v3;
                    level->m_faces.insert(make_pair(face, m_material));

                    face = & m_tmpFaces[cnt++];
                    face->vertexes.resize(3);
                    face->vertexes[0] = v2;
                    face->vertexes[1] = v3;
                    face->vertexes[2] = v4;
                    level->m_faces.insert(make_pair(face, m_material));
                }
            }
        }
    }
    
    NewtonTreeCollisionEndBuild(collision, 0);
    
    create(collision);
    
    if (Video::instance->m_haveVBO)
    {
        Video::glGenBuffersARB(5, (GLuint*)&m_buffers[0]);

        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[0]);
        Video::glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_uv.size() * sizeof(UV), &m_uv[0], GL_STATIC_DRAW_ARB);

        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[1]);
        Video::glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_normals.size() * sizeof(Vector), &m_normals[0], GL_STATIC_DRAW_ARB);

        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[2]);
        Video::glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_vertices.size() * sizeof(Vector), &m_vertices[0], GL_STATIC_DRAW_ARB);

        if (m_indices.size() != 0)
        {
            Video::glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_buffers[3]);
            Video::glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_indices.size() * sizeof(unsigned short), &m_indices[0], GL_STATIC_DRAW_ARB);
        }

        if (m_indices2.size() != 0)
        {
            Video::glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_buffers[4]);
            Video::glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_indices2.size() * sizeof(unsigned short), &m_indices2[0], GL_STATIC_DRAW_ARB);
        }

        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        Video::glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
}

void CollisionHMap::render() const
{
    if (m_indices.size() + m_indices2.size() == 0)
    {
        return;
    }

    m_material->bind();

    if (Video::instance->m_haveVBO)
    {
        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[0]);
        glTexCoordPointer(2, GL_FLOAT, sizeof(UV), NULL);

        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[1]);
        glNormalPointer(GL_FLOAT, sizeof(Vector), NULL);

        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[2]);
        glVertexPointer(3, GL_FLOAT, sizeof(Vector), NULL);

        if (m_indices.size() != 0)
        {
            Video::glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_buffers[3]);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_SHORT, NULL);
        }

        if (m_indices2.size() != 0)
        {
            if (Video::instance->m_shadowMap3ndPass)
            {
                Video::glActiveTextureARB(GL_TEXTURE1_ARB);
                glDisable(GL_TEXTURE_2D);
                Video::glActiveTextureARB(GL_TEXTURE0_ARB);
            }
            
            Video::glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_buffers[4]);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices2.size()), GL_UNSIGNED_SHORT, NULL);

            if (Video::instance->m_shadowMap3ndPass)
            {
                Video::glActiveTextureARB(GL_TEXTURE1_ARB);
                glEnable(GL_TEXTURE_2D);
                Video::glActiveTextureARB(GL_TEXTURE0_ARB);
            }
        }

        Video::glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
    else
    {
        glTexCoordPointer(2, GL_FLOAT, sizeof(UV), &m_uv[0]);
        glNormalPointer(GL_FLOAT, sizeof(Vector), &m_normals[0]);
        glVertexPointer(3, GL_FLOAT, sizeof(Vector), &m_vertices[0]);

        if (m_indices.size() != 0)
        {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_SHORT, &m_indices[0]);
        }

        if (m_indices2.size() != 0)
        {
            if (Video::instance->m_shadowMap3ndPass)
            {
                Video::glActiveTextureARB(GL_TEXTURE1_ARB);
                glDisable(GL_TEXTURE_2D);
                Video::glActiveTextureARB(GL_TEXTURE0_ARB);
            }
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices2.size()), GL_UNSIGNED_SHORT, &m_indices2[0]);
            if (Video::instance->m_shadowMap3ndPass)
            {
                Video::glActiveTextureARB(GL_TEXTURE1_ARB);
                glEnable(GL_TEXTURE_2D);
                Video::glActiveTextureARB(GL_TEXTURE0_ARB);
            }
        }
    }
}

CollisionHMap::~CollisionHMap()
{
    if (Video::instance->m_haveVBO)
    {
        Video::glDeleteBuffersARB(5, (GLuint*)&m_buffers[0]);
    }
}

float CollisionHMap::getHeight(float x, float z) const
{
    float x0 = (x + m_size/2.0f) * (m_realCount-1) / m_size;
    float z0 = (z + m_size/2.0f) * (m_realCount-1) / m_size;
    
    x0 = std::min(std::max(x0, 0.0f), static_cast<float>(m_realCount-2));
    z0 = std::min(std::max(z0, 0.0f), static_cast<float>(m_realCount-2));
    
    int ix = static_cast<int>(std::floor(x0));
    int iz = static_cast<int>(std::floor(z0));

    x0 -= static_cast<float>(ix);
    z0 -= static_cast<float>(iz);
  
    Vector v1;
    Vector v2;
    Vector v3;

    if (x0+z0 <= 1.0f)
    {
        // lower triangle
        v1 = m_vertices[m_realCount * iz + ix];
        v2 = m_vertices[m_realCount * (iz+1) + ix];
        v3 = m_vertices[m_realCount * iz + ix+1];
    }
    else
    {
        // upper triangle
        v1 = m_vertices[m_realCount * (iz+1) + (ix+1)];
        v2 = m_vertices[m_realCount * (iz+1) + ix];
        v3 = m_vertices[m_realCount * iz + ix+1];
    }
   
    Vector s1 = v2 - v1;
    Vector s2 = v3 - v1;
    s1.norm();
    s2.norm();
    Vector n = s1 ^ s2;
    n.norm();
    const float D = - (n % v1);

    return -(n.x * x + n.z * z + D)/n.y;
}

void CollisionHMap::renderTri(float x, float z) const
{
    if (Input::instance->key('`'))
    {
        float x0 = (x + m_size/2.0f) * (m_realCount-1) / m_size;
        float z0 = (z + m_size/2.0f) * (m_realCount-1) / m_size;
        
        x0 = std::min(std::max(x0, 0.0f), static_cast<float>(m_realCount-2));
        z0 = std::min(std::max(z0, 0.0f), static_cast<float>(m_realCount-2));
        
        int ix = static_cast<int>(std::floor(x0));
        int iz = static_cast<int>(std::floor(z0));

        x0 -= static_cast<float>(ix);
        z0 -= static_cast<float>(iz);
      
        Vector v1;
        Vector v2;
        Vector v3;

        if (x0+z0 <= 1.0f)
        {
            // lower triangle
            v1 = m_vertices[m_realCount * iz + ix];
            v2 = m_vertices[m_realCount * (iz+1) + ix];
            v3 = m_vertices[m_realCount * iz + ix+1];
        }
        else
        {
            // upper triangle
            v1 = m_vertices[m_realCount * (iz+1) + (ix+1)];
            v2 = m_vertices[m_realCount * (iz+1) + ix];
            v3 = m_vertices[m_realCount * iz + ix+1];
        }

        glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_CURRENT_BIT);
        
        glDisable(GL_CULL_FACE);
        glDisable(GL_TEXTURE_2D);
        glPolygonMode(GL_FRONT_AND_BACK , GL_FILL);
        glColor3f(1,1,1);

        glBegin(GL_TRIANGLES);
        glVertex3fv(v1.v);
        glVertex3fv(v2.v);
        glVertex3fv(v3.v);
        glEnd();

        glPopAttrib();
    }
}
