#include "mesh.h"

Mesh::Mesh(GLenum mode, bool indexed) :
    m_mode(mode),
    m_texcoordOffset(0),
    m_verticesOffset(0),
    m_indexed(indexed)
{
}

void Mesh::init()
{
    m_indicesCount = static_cast<GLsizei>(m_indices.size());

    if (Video::instance->m_haveVBO)
    {
        glGenBuffersARB( (m_indexed ? 2 : 1), (GLuint*)m_buffers);

        m_texcoordOffset = static_cast<GLuint>(m_normals.size() * sizeof(Vector));
        m_verticesOffset = m_texcoordOffset + static_cast<GLuint>(m_texcoords.size() * sizeof(Vector));

        GLuint size = m_verticesOffset + static_cast<GLuint>(m_vertices.size() * sizeof(Vector));

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[0]);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, NULL, GL_STATIC_DRAW_ARB);

        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, m_normals.size() * sizeof(Vector), &m_normals[0]);
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, m_texcoordOffset, m_texcoords.size() * sizeof(UV), &m_texcoords[0]);
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, m_verticesOffset, m_vertices.size() * sizeof(Vector), &m_vertices[0]);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        if (m_indexed)
        {
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_buffers[1]);
            glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_indices.size() * sizeof(unsigned short), &m_indices[0], GL_STATIC_DRAW_ARB);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

            m_indices.clear();
        }
        else
        {
            m_indicesCount = static_cast<GLsizei>(m_vertices.size());
        }

        m_vertices.clear();
        m_normals.clear();
        m_texcoords.clear();
    }
}

Mesh::~Mesh()
{
    if (Video::instance->m_haveVBO)
    {
        glDeleteBuffersARB( (m_indexed ? 2 : 1), (GLuint*)m_buffers);
    }
}

void Mesh::render() const
{
    if (Video::instance->m_haveVBO)
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[0]);
        glNormalPointer(GL_FLOAT, sizeof(Vector), NULL);
        glTexCoordPointer(2, GL_FLOAT, sizeof(UV), (char*)NULL + m_texcoordOffset);
        glVertexPointer(3, GL_FLOAT, sizeof(Vector), (char*)NULL + m_verticesOffset);

        if (m_indexed)
        {
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_buffers[1]);
            glDrawElements(m_mode, m_indicesCount, GL_UNSIGNED_SHORT, NULL);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        }
        else
        {
            glDrawArrays(m_mode, 0, m_indicesCount);
        }

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
    else
    {
        glNormalPointer(GL_FLOAT, sizeof(Vector), &m_normals[0]);
        glTexCoordPointer(2, GL_FLOAT, sizeof(UV), &m_texcoords[0]);
        glVertexPointer(3, GL_FLOAT, sizeof(Vector), &m_vertices[0]);

        if (m_indexed)
        {
            glDrawElements(m_mode, m_indicesCount, GL_UNSIGNED_SHORT, &m_indices[0]);
        }
        else
        {
            glDrawArrays(m_mode, 0, m_indicesCount);
        }

    }
}

CubeMesh::CubeMesh(const Vector& size) : Mesh(GL_TRIANGLES, false)
{
    // -0.5 .. 0.5
    static const float vertices[][3] = {
        /* 0 */ { -0.5, -0.5, -0.5 },
        /* 1 */ {  0.5, -0.5, -0.5 },
        /* 2 */ {  0.5, -0.5,  0.5 },
        /* 3 */ { -0.5, -0.5,  0.5 },

        /* 4 */ { -0.5,  0.5, -0.5 },
        /* 5 */ {  0.5,  0.5, -0.5 },
        /* 6 */ {  0.5,  0.5,  0.5 },
        /* 7 */ { -0.5,  0.5,  0.5 },
    };

    static const int faces[][6] = {
        { 0, 1, 2, 3, 0, 2 }, // bottom
        { 4, 7, 6, 5, 4, 6 }, // up
        { 4, 5, 1, 0, 4, 1 }, // front
        { 6, 7, 3, 2, 6, 3 }, // back
        { 7, 4, 0, 3, 7, 0 }, // left
        { 5, 6, 2, 1, 5, 2 }, // right
    };
    
    static const float normals[][3] = {
        {  0.0, -1.0,  0.0 }, // bottom
        {  0.0,  1.0,  0.0 }, // up
        {  0.0,  0.0, -1.0 }, // front
        {  0.0,  0.0,  1.0 }, // back
        { -1.0,  0.0,  0.0 }, // left
        {  1.0,  0.0,  0.0 }, // right
    };

    static const float uv[][2] = {
        { 1.0, 0.0 },
        { 0.0, 0.0 },
        { 0.0, 1.0 },
        { 1.0, 1.0 },
        { 1.0, 0.0 },
        { 0.0, 1.0 },
    };

    for (size_t i = 0; i < sizeOfArray(faces); i++)
    {
        for (int k=0; k<6; k++)
        {
            m_normals.push_back(Vector(normals[i][0], normals[i][1], normals[i][2]));
            m_texcoords.push_back(UV(uv[k][0], uv[k][1]));
            m_vertices.push_back(size * Vector(vertices[faces[i][k]][0], vertices[faces[i][k]][1], vertices[faces[i][k]][2]));
        }
    }

    init();
}

SphereMesh::SphereMesh(const Vector& radius, int stacks, int slices) : Mesh(GL_TRIANGLE_STRIP, true)
{   
    for (int stackNumber = 0; stackNumber <= stacks; ++stackNumber)
    {
        for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
        {
            float theta = stackNumber * M_PI / stacks;
            float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
            float sinTheta = std::sin(theta);
            float sinPhi = std::sin(phi);
            float cosTheta = std::cos(theta);
            float cosPhi = std::cos(phi);

            float s = static_cast<float>(sliceNumber)/static_cast<float>(slices);
            float t = 1.0f-static_cast<float>(stackNumber)/static_cast<float>(stacks);

            m_normals.push_back(Vector(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta));
            m_vertices.push_back(radius * m_normals.back());
            m_texcoords.push_back(UV(s, t));
        }
    }

    for (int stackNumber = 0; stackNumber < stacks; ++stackNumber)
    {
        for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
        {
            m_indices.push_back( stackNumber*(slices+1) + sliceNumber);
            m_indices.push_back( (stackNumber+1)*(slices+1) + sliceNumber);
        }
    }

    init();
}

CylinderMesh::CylinderMesh(float radius, float height, int stacks, int slices) : Mesh(GL_TRIANGLE_STRIP, true)
{
    // bottom disc
    m_normals.push_back(-Vector::X);
    m_vertices.push_back(Vector(0.0f, 0.0f, 0.0f));
    m_texcoords.push_back(UV(0.5f, 0.5f));
    
    for (int sliceNumber = slices; sliceNumber >= 0; --sliceNumber)
    {
        float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        m_normals.push_back(-Vector::X);
        m_vertices.push_back(Vector(0.0f, radius * cosPhi, radius * sinPhi));
        m_texcoords.push_back(UV(cosPhi, sinPhi));
    }

    // cylinder
    for (int stackNumber = 0; stackNumber <= stacks; ++stackNumber)
    {
        for (int sliceNumber = slices; sliceNumber >=0 ; --sliceNumber)
        {
            float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            float s = static_cast<float>(sliceNumber)/static_cast<float>(slices);
            float t = 1.0f-static_cast<float>(stackNumber)/static_cast<float>(stacks);

            m_normals.push_back(Vector(0.0f, cosPhi, sinPhi));
            m_vertices.push_back(Vector(height * stackNumber / stacks, radius * cosPhi, radius * sinPhi));
            m_texcoords.push_back(UV(s, t));
        }
    }

    // upper disc
    for (int sliceNumber = slices; sliceNumber >= 0; --sliceNumber)
    {
        float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        m_normals.push_back(Vector::X);
        m_vertices.push_back(Vector(height, radius * cosPhi, radius * sinPhi));
        m_texcoords.push_back(UV(cosPhi, sinPhi));
    }
    m_normals.push_back(Vector::X);
    m_vertices.push_back(Vector(height, 0.0f, 0.0f));
    m_texcoords.push_back(UV(0.5f, 0.5f));


    // bottom disc
    for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
    {
        m_indices.push_back( 0 );
        m_indices.push_back( 1 + sliceNumber );
    }

    // cylinder
    for (int stackNumber = 0; stackNumber < stacks; ++stackNumber)
    {
        for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
        {
            m_indices.push_back( (1+slices+1) + stackNumber*(slices+1) + sliceNumber);
            m_indices.push_back( (1+slices+1) + (stackNumber+1)*(slices+1) + sliceNumber);
        }
    }

    // upper disc
    for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
    {
        m_indices.push_back( (1+slices+1) + (stacks+1)*(slices+1) + sliceNumber );
        m_indices.push_back( (1+slices+1) + (stacks+1)*(slices+1) + (slices+1) );
    }

    init();
}

ConeMesh::ConeMesh(float radius, float height, int stacks, int slices) : Mesh(GL_TRIANGLE_STRIP, true)
{
    // bottom disc
    m_normals.push_back(-Vector::X);
    m_vertices.push_back(Vector(0.0f, 0.0f, 0.0f));
    m_texcoords.push_back(UV(0.5f, 0.5f));
    
    for (int sliceNumber = slices; sliceNumber >= 0; --sliceNumber)
    {
        float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        m_normals.push_back(-Vector::X);
        m_vertices.push_back(Vector(0.0f, radius * cosPhi, radius * sinPhi));
        m_texcoords.push_back(UV(cosPhi, sinPhi));
    }

    // cone
    for (int stackNumber = 0; stackNumber <= stacks; ++stackNumber)
    {
        for (int sliceNumber = slices; sliceNumber >=0 ; --sliceNumber)
        {
            float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);
            float s = static_cast<float>(sliceNumber)/static_cast<float>(slices);
            float t = 1.0f-static_cast<float>(stackNumber)/static_cast<float>(stacks);

            m_normals.push_back(Vector(0.0f, cosPhi, sinPhi));
            m_vertices.push_back(Vector(height * (1.0f - t), radius * cosPhi * t, radius * sinPhi * t));
            m_texcoords.push_back(UV(s, t));
        }
    }

    // bottom disc
    for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
    {
        m_indices.push_back( 0 );
        m_indices.push_back( 1 + sliceNumber );
    }

    // cone
    for (int stackNumber = 0; stackNumber < stacks; ++stackNumber)
    {
        for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
        {
            m_indices.push_back( (1+slices+1) + stackNumber*(slices+1) + sliceNumber);
            m_indices.push_back( (1+slices+1) + (stackNumber+1)*(slices+1) + sliceNumber);
        }
    }

    init();
}
