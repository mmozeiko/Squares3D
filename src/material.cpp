#include "material.h"
#include "shader.h"
#include "xml.h"
#include "video.h"
#include "texture.h"
#include "level.h"
#include "world.h"

Material::Material(const XMLnode& node) :
    m_id(),
    //m_shader(NULL),
    m_cAmbient(0.2f, 0.2f, 0.2f),
    m_cSpecular(0.0f, 0.0f, 0.0f),
    m_cEmission(0.0f, 0.0f, 0.0f),
    m_cShine(0.0f),
    m_texture(0)
    //m_textureBump(0)
{
    m_id = node.getAttribute("id");

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "texture2D")
        {
            m_texture = Video::instance->loadTexture(node.getAttribute("name"));
        }
        else if (node.name == "colors")
        {
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "ambient")
                {
                    m_cAmbient = node.getAttributesInVector("rgb");
                }
                else if (node.name == "specular")
                {
                    m_cSpecular = node.getAttributesInVector("rgb");
                }
                else if (node.name == "emission")
                {
                    m_cEmission = node.getAttributesInVector("rgb");
                }
                else if (node.name == "shine")
                {
                    m_cShine = cast<float>(node.value);
                }
                else
                {
                    throw Exception("Invalid color, unknown node - " + node.name);
                }
            }
        }
        else
        {
            throw Exception("Invalid material, unknown node - " + node.name);
        }
    }
        //else if (node.name == "shader")
        //{
        //    std::string name = getAttribute(node, "name");
        //    m_shader = Video::instance->loadShader(name + ".vp", name + ".fp");
        //}
}

void Material::bind() const
{
    //Video::glActiveTextureARB(GL_TEXTURE0_ARB);
    m_texture->bind();

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, m_cAmbient.v);
    glMaterialfv(GL_FRONT, GL_SPECULAR, m_cSpecular.v);
    glMaterialfv(GL_FRONT, GL_EMISSION, m_cEmission.v);
    glMaterialf(GL_FRONT, GL_SHININESS, m_cShine);

    /*
    if (Video::instance->m_haveShaders && (m_shader != NULL))
    {
        Video::glActiveTextureARB(GL_TEXTURE1_ARB);
        m_textureBump->begin();
        Video::instance->begin(m_shader);
    }
    */
}

/*
void Material::unbind() const
{
    if (Video::instance->m_haveShaders && (m_shader != NULL))
    {
        Video::instance->end(m_shader);
        Video::glActiveTextureARB(GL_TEXTURE1_ARB);
        m_textureBump->end();
    }

    Video::glActiveTextureARB(GL_TEXTURE0_ARB);
}
*/
