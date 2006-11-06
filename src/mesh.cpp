#include "mesh.h"

Mesh::Mesh(GLenum mode, bool normals) : m_mode(mode), m_haveNormals(normals)
{
}

void Mesh::init()
{
    m_indicesCount = static_cast<GLsizei>(m_indices.size());
    if (Video::instance->m_haveVBO)
    {
        glGenBuffersARB( 4, (GLuint*)m_buffers);

        if (m_haveNormals)
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[0]);
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_normals.size() * sizeof(Vector), &m_normals[0], GL_STATIC_DRAW_ARB);
        }

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[1]);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_texcoords.size() * sizeof(UV), &m_texcoords[0], GL_STATIC_DRAW_ARB);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[2]);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_vertices.size() * sizeof(Vector), &m_vertices[0], GL_STATIC_DRAW_ARB);

        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_buffers[3]);
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_indices.size() * sizeof(unsigned short), &m_indices[0], GL_STATIC_DRAW_ARB);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

        m_vertices.clear();
        m_normals.clear();
        m_texcoords.clear();
        m_indices.clear();
    }
}

Mesh::~Mesh()
{
    if (Video::instance->m_haveVBO)
    {
        glDeleteBuffersARB( 4, (GLuint*)m_buffers);
    }
}

void Mesh::render() const
{
    if (Video::instance->m_haveVBO)
    {
        if (m_haveNormals)
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[0]);
            glNormalPointer(GL_FLOAT, sizeof(Vector), NULL);
        }
        else
        {
            glDisableClientState(GL_NORMAL_ARRAY);
        }

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[1]);
        glTexCoordPointer(2, GL_FLOAT, sizeof(UV), NULL);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffers[2]);
        glVertexPointer(3, GL_FLOAT, sizeof(Vector), NULL);

        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_buffers[3]);
        glDrawElements(m_mode, m_indicesCount, GL_UNSIGNED_SHORT, NULL);

        if (!m_haveNormals)
        {
            glEnableClientState(GL_NORMAL_ARRAY);
        }

        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
    else
    {
        glVertexPointer(3, GL_FLOAT, sizeof(Vector), &m_vertices[0]);
        glTexCoordPointer(2, GL_FLOAT, sizeof(UV), &m_texcoords[0]);
        
        if (m_haveNormals)
        {
            glNormalPointer(GL_FLOAT, sizeof(Vector), &m_normals[0]);
        }
        else
        {
            glDisableClientState(GL_NORMAL_ARRAY);
        }

        glDrawElements(m_mode, m_indicesCount, GL_UNSIGNED_SHORT, &m_indices[0]);

        if (!m_haveNormals)
        {
            glEnableClientState(GL_NORMAL_ARRAY);
        }
    }
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
    m_normals.push_back(-Vector::Z);
    m_vertices.push_back(Vector(0.0f, 0.0f, 0.0f));
    m_texcoords.push_back(UV(0.5f, 0.5f));
    
    for (int sliceNumber = slices; sliceNumber >= 0; --sliceNumber)
    {
        float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        m_normals.push_back(-Vector::Z);
        m_vertices.push_back(Vector(radius * cosPhi, radius * sinPhi, 0.0f));
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

            m_normals.push_back(Vector(cosPhi, sinPhi, 0.0f));
            m_vertices.push_back(Vector(radius * cosPhi, radius * sinPhi, height * stackNumber / stacks));
            m_texcoords.push_back(UV(s, t));
        }
    }

    // upper disc
    for (int sliceNumber = slices; sliceNumber >= 0; --sliceNumber)
    {
        float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        m_normals.push_back(Vector::Z);
        m_vertices.push_back(Vector(radius * cosPhi, radius * sinPhi, height));
        m_texcoords.push_back(UV(cosPhi, sinPhi));
    }
    m_normals.push_back(Vector::Z);
    m_vertices.push_back(Vector(0.0f, 0.0f, height));
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
    m_normals.push_back(-Vector::Z);
    m_vertices.push_back(Vector(0.0f, 0.0f, 0.0f));
    m_texcoords.push_back(UV(0.5f, 0.5f));
    
    for (int sliceNumber = slices; sliceNumber >= 0; --sliceNumber)
    {
        float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        m_normals.push_back(-Vector::Z);
        m_vertices.push_back(Vector(radius * cosPhi, radius * sinPhi, 0.0f));
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

            m_normals.push_back(Vector(cosPhi, sinPhi, 0.0f));
            m_vertices.push_back(Vector(radius * cosPhi * t, radius * sinPhi * t, height * (1.0f - t)));
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
