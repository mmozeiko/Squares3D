#include "grass.h"
#include "level.h"
#include "texture.h"
#include "material.h"
#include "random.h"
#include "geometry.h"
#include "config.h"
#include "video_ext.h"

Grass::Grass(const Level* level) : m_time(0.0f), m_count(0), m_count2(0), m_grassTex(NULL)
{
    // 1.0f, 2.0f, 4.0f
    float grass_density = static_cast<float>(1 << Config::instance->m_video.grass_density);
    
    //size of grass face
    static const float SIZE = 0.4f;

    const Vector lower(-3.2f, 0.0f, -3.2f);
    const Vector upper(3.2f, 0.0f, 3.2f);
  
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
        float count = grass_density * area;

        if (count >= 1.0f || (count < 1.0f && Random::getFloat() < count))
        {
            for (int n = 0; n < count; n++)
            {
                float s = Random::getFloat();
                float t = Random::getFloat();
                
                // mapping from [0,1]x[0,1] square to triangle
                t = sqrt(t);
                float a = 1 - t;
                float b = (1 - s)*t;
                float c = s * t;

                Vector v(a * face->vertexes[0] + b * face->vertexes[1] + c * face->vertexes[2]);

                if (!(isPointInRectangle(v, lower, upper)))
                {

                    Matrix trM = Matrix::translate(v) * Matrix::rotateY(Random::getFloatN(2*M_PI));

                    vector<GrassFace>* faces;
                    if (isPointInRectangle(v, g_fieldLower, g_fieldUpper))
                    {
                        faces = &m_faces;
                    }
                    else
                    {
                        faces = &m_faces2;
                    }
                    
                    faces->push_back(GrassFace( UV(0.0f, 0.0f), trM * Vector(-SIZE/2, 0.0f, 0.0f)) );
                    faces->push_back(GrassFace( UV(1.0f, 0.0f), trM * Vector(+SIZE/2, 0.0f, 0.0f)) );
                    faces->push_back(GrassFace( UV(1.0f, 1.0f), trM * Vector(+SIZE/2, SIZE, 0.0f)) );
                    faces->push_back(GrassFace( UV(0.0f, 1.0f), trM * Vector(-SIZE/2, SIZE, 0.0f)) );

                    faces->push_back(GrassFace( UV(0.0f, 0.0f), trM * Vector(0.0f, 0.0f, -SIZE/2)) );
                    faces->push_back(GrassFace( UV(1.0f, 0.0f), trM * Vector(0.0f, 0.0f, +SIZE/2)) );
                    faces->push_back(GrassFace( UV(1.0f, 1.0f), trM * Vector(0.0f, SIZE, +SIZE/2)) );
                    faces->push_back(GrassFace( UV(0.0f, 1.0f), trM * Vector(0.0f, SIZE, -SIZE/2)) );
                }

            }
        }
    }

    m_count = m_faces.size();
    m_count2 = m_faces2.size();

    if (Video::instance->m_haveVBO)
    {
        glGenBuffersARB(2, (GLuint*)&m_buffer[0]);

        if (m_count > 0)
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffer[0]);
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(GrassFace)*m_count, &m_faces[0], GL_DYNAMIC_DRAW_ARB);
        }

        if (m_count2 > 0)
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffer[1]);
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(GrassFace)*m_count2, &m_faces2[0], GL_DYNAMIC_DRAW_ARB);
        }

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }

    m_grassTex = Video::instance->loadTexture("grassThingy");
    m_grassTex->setWrap(Texture::Clamp);
}

Grass::~Grass()
{
    if (Video::instance->m_haveVBO)
    {
        glDeleteBuffersARB(2, (GLuint*)&m_buffer[0]);
    }
}

void Grass::update(float delta)
{
    if (m_count+m_count2 == 0)
    {
        return;
    }

    m_time += delta;

    float n = 0.25f*sin(m_time * M_PI / 6.0f);

    for (size_t i=0; i<m_count; i+=4)
    {
        m_faces[i+2].uv.u = 1.0f + n;
        m_faces[i+3].uv.u = 0.0f + n;
    }
    
    for (size_t i=0; i<m_count2; i+=4)
    {
        m_faces2[i+2].uv.u = 1.0f + n;
        m_faces2[i+3].uv.u = 0.0f + n;
    }

    if (Video::instance->m_haveVBO)
    {
        if (m_count != 0)
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffer[0]);
            glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(GrassFace)*m_count, &m_faces[0]);
        }

        if (m_count2 != 0)
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffer[1]);
            glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(GrassFace)*m_count2, &m_faces2[0]);
        }

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
}

void Grass::render() const
{
    if (m_count + m_count2 == 0)
    {
        return;
    }

    m_grassTex->bind();

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glAlphaFunc(GL_GEQUAL, 0.3f);
    glEnable(GL_ALPHA_TEST);

    if (Video::instance->m_haveVBO)
    {
        if (m_count != 0)
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffer[0]);
            glInterleavedArrays(GL_T2F_V3F, sizeof(GrassFace), NULL);
            glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(m_count));
        }

        if (m_count2 != 0)
        {

            if (Video::instance->m_shadowMap3ndPass)
            {
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glDisable(GL_TEXTURE_2D);   
                glActiveTextureARB(GL_TEXTURE0_ARB);
            }

            glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_buffer[1]);
            glInterleavedArrays(GL_T2F_V3F, sizeof(GrassFace), NULL);
            glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(m_count2));

            if (Video::instance->m_shadowMap3ndPass)
            {
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glEnable(GL_TEXTURE_2D);
                glActiveTextureARB(GL_TEXTURE0_ARB);
            }
        }

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
    else
    {
        if (m_count != 0)
        {
            glInterleavedArrays(GL_T2F_V3F, sizeof(GrassFace), &m_faces[0]);
            glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(m_count));
        }

        if (m_count2 != 0)
        {
            if (Video::instance->m_shadowMap3ndPass)
            {
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glDisable(GL_TEXTURE_2D);
                glActiveTextureARB(GL_TEXTURE0_ARB);
            }

            glInterleavedArrays(GL_T2F_V3F, sizeof(GrassFace), &m_faces2[0]);
            glDrawArrays(GL_QUADS, 0, static_cast<GLsizei>(m_count2));

            if (Video::instance->m_shadowMap3ndPass)
            {
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glEnable(GL_TEXTURE_2D);
                glActiveTextureARB(GL_TEXTURE0_ARB);
            }
        }

    }

    glPopAttrib();

    glEnableClientState(GL_NORMAL_ARRAY);
}
