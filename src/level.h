#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "common.h"
#include "vmath.h"
#include "video.h"
#include "newton.h"

class XMLnode;
class Game;

namespace LevelObjects
{
    class Material;    
    class Collision;
    class Body;

    typedef map<string, Material*>  MaterialsMap;
    typedef map<string, Collision*> CollisionsMap;
    typedef map<string, Body*> BodiesMap;

    class Material
    {
        friend class Level;
    public:
        string m_id;        // ""
        string m_shaderName; // ""
        Vector m_cAmbient;  // (0.2f, 0.2f, 0.2f, 1.0f)
        Vector m_cDiffuse;  // (0.8f, 0.8f, 0.8f, 1.0f)
        Vector m_cSpecular; // (0.0f, 0.0f, 0.0f)
        Vector m_cEmission; // (0.0f, 0.0f, 0.0f, 1.0f)
        float  m_cShine;    // 0.0f

        void render(const Video* video) const;
    private: 
        Material(const XMLnode& node, const Game* game);
        unsigned int m_texture;
        unsigned int m_textureBump;
    };

    class Collision
    {
        friend class Body;
    public:
        static Collision* create(const XMLnode& node, const Game* game);
        
        virtual void render(const Video* video, const MaterialsMap* materials) const = 0;

        NewtonCollision*  m_newtonCollision;
        virtual ~Collision();
    
    protected:
        Collision(const XMLnode& node, const Game* game);

        void create(NewtonCollision* collision);
        void create(NewtonCollision* collision, float mass);

    private:
        NewtonWorld* m_newtonWorld;
        Vector       m_origin;
        Vector       m_inertia;
        float        m_mass;
    };

    class Body
    {
        friend class Level;

    public:
        void prepare();
        
    private:
        Body(const XMLnode& node, const Game* game);
        ~Body();

        void render(const Video* video, const MaterialsMap* materials);

        NewtonBody*     m_newtonBody;
        Matrix          m_matrix;
        float           m_totalMass;
        Vector          m_totalInertia;
        NewtonWorld*    m_newtonWorld;
        set<Collision*> m_collisions;

        void onSetForceAndTorque();
        static void onSetForceAndTorque(const NewtonBody* newtonBody);
    };

    class Level
    {
    public:
        Level(const Game* game);
        ~Level();
        void load(const string& levelFile);
        void render(const Video* video) const;
        void prepare();

        BodiesMap     m_bodies;
        CollisionsMap m_collisions;
        MaterialsMap  m_materials;
    private:
        const Game* m_game;
    };

}
#endif