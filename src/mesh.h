#ifndef __MESH_H__
#define __MESH_H__

#include "common.h"
#include "vmath.h"
#include "video.h"

typedef vector<Vector> VectorVector;
typedef vector<UV>     TexCoords;

class Mesh
{
public:
    Mesh(GLenum mode, bool indexed);
    ~Mesh();
  
    void render() const;

protected:
    void init();

    VectorVector m_normals; 
    TexCoords    m_texcoords;
    VectorVector m_vertices;

    UShortVector m_indices;

private:
    GLuint  m_buffers[2];
    
    GLuint  m_texcoordOffset;
    GLuint  m_verticesOffset;

    GLenum  m_mode;
    GLsizei m_indicesCount;
    bool    m_indexed;
};

class CubeMesh : public Mesh
{
public:
    CubeMesh(const Vector& size);
};

class SphereMesh : public Mesh
{
public:
    SphereMesh(const Vector& radius, int stacks, int slices);
};

class CylinderMesh : public Mesh
{
public:
    CylinderMesh(float radius, float height, int stacks, int slices);
};

class ConeMesh : public Mesh
{
public:
    ConeMesh(float radius, float height, int stacks, int slices);
};

#endif
