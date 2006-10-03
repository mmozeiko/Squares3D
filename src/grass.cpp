#include "grass.h"
#include "level.h"
#include "texture.h"
#include "material.h"
#include "random.h"

static const float DENSITY = 1.0f; // 1 2 4

Grass::Grass(const Level* level) : m_grassTex(NULL), m_count(0), m_time(0.0f)
{
    for each_const(FaceSet, level->m_faces, iter)
    {
        const Face*     face = iter->first;
        const Material* material = iter->second;
        if (material == NULL || material->m_id != "grass")
        {
            continue;
        }
        
        Vector side1 = face->vertexes[1] - face->vertexes[0];
        Vector side2 = face->vertexes[2] - face->vertexes[0];
        Vector areaV = side1 ^ side2;
        float area = areaV.magnitude();
        float count = DENSITY * area;

        if (count < 1.0f && Random::getFloat() < count)
        for (int n = 0; n < count; n++)
        {
            float sum = 0.0f;

            Vector v = face->vertexes[0] + side1 * (Random::getFloat()) + side2 * (Random::getFloat());

            Matrix trM = Matrix::translate(v) * Matrix::rotateY(Random::getFloatN(2*M_PI));

            m_faces.push_back(GrassFace( UV(0.0f, 0.0f), trM * Vector(-0.2f, 0.0f, 0.0f)) );
            m_faces.push_back(GrassFace( UV(1.0f, 0.0f), trM * Vector(+0.2f, 0.0f, 0.0f)) );
            m_faces.push_back(GrassFace( UV(1.0f, 1.0f), trM * Vector(+0.2f, 0.4f, 0.0f)) );
            m_faces.push_back(GrassFace( UV(0.0f, 1.0f), trM * Vector(-0.2f, 0.4f, 0.0f)) );

            m_faces.push_back(GrassFace( UV(0.0f, 0.0f), trM * Vector(0.0f, 0.0f, -0.2f)) );
            m_faces.push_back(GrassFace( UV(1.0f, 0.0f), trM * Vector(0.0f, 0.0f, +0.2f)) );
            m_faces.push_back(GrassFace( UV(1.0f, 1.0f), trM * Vector(0.0f, 0.4f, +0.2f)) );
            m_faces.push_back(GrassFace( UV(0.0f, 1.0f), trM * Vector(0.0f, 0.4f, -0.2f)) );

        }
    }

    m_count = m_faces.size();
   
    if (Video::instance->m_haveVBO)
    {
        Video::glGenBuffersARB(1, &m_buffer);

        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffer);
        Video::glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(GrassFace)*m_count, &m_faces[0], GL_DYNAMIC_DRAW_ARB);
        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }

    m_grassTex = Video::instance->loadTexture("grassWalpha");
    m_grassTex->setWrap(Texture::Clamp);
}

Grass::~Grass()
{
    if (Video::instance->m_haveVBO)
    {
        Video::glDeleteBuffersARB(1, &m_buffer);
    }
}

void Grass::update(float delta)
{
    m_time += delta;

    float n = 0.4f*sin(m_time * M_PI / 6.0f);

    for (size_t i=0; i<m_count; i+=4)
    {
        m_faces[i+2].uv.u = 1.0f + n;
        m_faces[i+3].uv.u = 0.0f + n;
    }

    if (Video::instance->m_haveVBO)
    {
        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffer);
        Video::glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(GrassFace)*m_count, &m_faces[0]);
        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
}

void Grass::render() const
{
    m_grassTex->bind();

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (Video::instance->m_haveVBO)
    {
        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffer);
        glInterleavedArrays(GL_T2F_V3F, sizeof(GrassFace), NULL);
        glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(m_count)); 
        Video::glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
    else
    {
        glInterleavedArrays(GL_T2F_V3F, sizeof(GrassFace), &m_faces[0]);
        glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(m_count));
    }

    glPopAttrib();

    glEnableClientState(GL_NORMAL_ARRAY);
}
