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

    typedef map<string, Material*> MaterialsMap;
    typedef map<string, Collision*> CollisionsMap;

    class Material
    {
        friend class Level;
    public:
        string m_id;        // ""
        string m_texPath;   // ""
        Vector m_cAmbient;  // (0.2f, 0.2f, 0.2f, 1.0f)
        Vector m_cDiffuse;  // (0.8f, 0.8f, 0.8f, 1.0f)
        Vector m_cSpecular; // (0.0f, 0.0f, 0.0f)
        Vector m_cEmission; // (0.0f, 0.0f, 0.0f, 1.0f)
        float  m_cShine;    // 0.0f

        void render(const Video* video) const;
    private: 
        Material(const XMLnode& node, const Game* game);
        unsigned int m_texture;
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
        void create(NewtonCollision* collision, float mass, const Vector& position);

    private:
        NewtonWorld*      m_newtonWorld;
        Vector            m_inertia;
        float             m_mass;
    };

    class Body
    {
        friend class Level;

    public:
        void prepare();
        
        string          m_id;         // ""
        string          m_material;   // ""
        //Vector          m_position;   // (0.0f, 0.0f, 0.0f)
        //Vector          m_rotation;   // (0.0f, 0.0f, 0.0f)
        set<Collision*> m_collisions;

    private:
        Body(const XMLnode& node, const Game* game);
        ~Body();
        void render(const Video* video, const MaterialsMap* materials);

        NewtonBody*     m_newtonBody;
        Matrix          m_matrix;
        float           m_totalMass;
        Vector          m_totalInertia;
        NewtonWorld*    m_newtonWorld;

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

        set<Body*>    m_bodies;
        CollisionsMap m_collisions;
        MaterialsMap  m_materials;
    private:
        const Game* m_game;
    };

    class CollisionBox : public Collision
    {
        friend class Collision;
    public:
        void render(const Video* video, const MaterialsMap* materials) const;

        Vector m_size;      // (1.0f, 1.0f, 1.0f)
        //Vector m_position;  // (0.0f, 0.0f, 0.0f)
        //Vector m_rotation;  // (0.0f, 0.0f, 0.0f)
        bool   m_hasOffset; // false

    private:
        CollisionBox(const XMLnode& node, const Game* game);

        Matrix m_matrix;
    };

    class CollisionTree : public Collision
    {
        friend class Collision;
    public:
        void render(const Video* video, const MaterialsMap* materials) const;

        vector<Face>   m_faces;
        vector<string> m_materials;
    private:
        CollisionTree(const XMLnode& node, const Game* game);
    };

}
#endif