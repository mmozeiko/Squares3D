#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "common.h"
#include "vmath.h"
#include "video.h"

class XMLnode;
class Game;

namespace LevelObjects
{
    class Material
    {
        friend class Level;
    public:
        string m_id; //""
        string m_texPath; //""
        Vector m_cAmbient; //(0.2f, 0.2f, 0.2f, 1.0f)
        Vector m_cDiffuse; //(0.8f, 0.8f, 0.8f, 1.0f)
        Vector m_cSpecular; //(0.0f, 0.0f, 0.0f)
        Vector m_cEmission; //(0.0f, 0.0f, 0.0f, 1.0f)
        float  m_cShine; //0.0f

        void render(const Video* video) const;
    private: 
        Material(const XMLnode& node, const Game* game);
        unsigned int m_texture;
    };

    typedef map<string, Material*> MaterialsMap;

    class Collision
    {
        friend class Body;
    public:
        static Collision* create(const XMLnode& node);

        virtual void render(const Video* video, const MaterialsMap* materials) const = 0;
    protected:
        Collision(const XMLnode& node);
    };

    class Body
    {
        friend class Level;

    public:
        string          m_id; //""
        string          m_material; //""
        Vector          m_position; //(0.0f, 0.0f, 0.0f)
        Vector          m_rotation; //(0.0f, 0.0f, 0.0f)
        set<Collision*> m_collisions;
    private:
        void render(const Video* video, const MaterialsMap* materials) const;
        Body(const XMLnode& node);
        ~Body();
    };

    class Level
    {
    public:
        Level(const Game* game);
        ~Level();
        void load(const string& levelFile);
        void render(const Video* video) const;

        set<Body*>   m_bodies;
        MaterialsMap m_materials;
    private:
        const Game* m_game;
    };

    class CollisionBox : public Collision
    {
        friend class Collision;
    public:
        void render(const Video* video, const MaterialsMap* materials) const;

        float  m_mass; //1.0f
        Vector m_size; //(1.0f, 1.0f, 1.0f)
        Vector m_position; //(0.0f, 0.0f, 0.0f)
        Vector m_rotation; //(0.0f, 0.0f, 0.0f)
    private:
        CollisionBox(const XMLnode& node);
    };

    class CollisionTree : public Collision
    {
        friend class Collision;
    public:
        void render(const Video* video, const MaterialsMap* materials) const;

        vector<Face>   m_faces;
        vector<string> m_materials;
    private:
        CollisionTree(const XMLnode& node);
    };

}
#endif