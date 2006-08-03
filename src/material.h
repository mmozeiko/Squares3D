#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "common.h"
#include "vmath.h"

class Video;
class Game;
class Level;
class Shader;
class Texture;
class XMLnode;

class Material
{
    friend class Level;
public:
    string  m_id;        // ""
    Shader* m_shader;
    Vector  m_cAmbient;  // (0.2f, 0.2f, 0.2f, 1.0f)
    Vector  m_cDiffuse;  // (0.8f, 0.8f, 0.8f, 1.0f)
    Vector  m_cSpecular; // (0.0f, 0.0f, 0.0f)
    Vector  m_cEmission; // (0.0f, 0.0f, 0.0f, 1.0f)
    float   m_cShine;    // 0.0f

    int     m_newtonID;

    void enable(const Video* video) const;
    void disable(const Video* video) const;
private: 
    Material(const XMLnode& node, const Game* game);

    Texture* m_texture;
    Texture* m_textureBump;
    
    const Game* m_game;
};

#endif