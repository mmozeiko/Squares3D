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
    Mesh(GLenum mode, bool normals);
    ~Mesh();
  
    void render() const;

protected:
    void init();

    VectorVector m_vertices;
    VectorVector m_normals; 
    TexCoords    m_texcoords;

    UShortVector m_indices;

private:
    GLuint  m_buffers[4];
    bool    m_haveNormals;
    GLenum  m_mode;
    GLsizei m_indicesCount;
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
