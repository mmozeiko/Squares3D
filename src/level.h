#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "common.h"
#include "vmath.h"
#include "video.h"
#include "newton.h"

class XMLnode;
class Game;
class Shader;

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
        string  m_id;        // ""
        Shader* m_shader;
        Vector  m_cAmbient;  // (0.2f, 0.2f, 0.2f, 1.0f)
        Vector  m_cDiffuse;  // (0.8f, 0.8f, 0.8f, 1.0f)
        Vector  m_cSpecular; // (0.0f, 0.0f, 0.0f)
        Vector  m_cEmission; // (0.0f, 0.0f, 0.0f, 1.0f)
        float   m_cShine;    // 0.0f

        void enable(const Video* video) const;
        void disable(const Video* video) const;
    private: 
        Material(const XMLnode& node, Video* video);

        unsigned int m_texture;
        unsigned int m_textureBump;
    };

    class Collision
    {
        friend class Body;
    public:
        static Collision* create(const XMLnode& node, const NewtonWorld* newtonWorld, const MaterialsMap* materials);
        
        virtual void render(const Video* video) const = 0;

        NewtonCollision*  m_newtonCollision;
        virtual ~Collision();
    
    protected:
        Collision(const XMLnode& node, const NewtonWorld* newtonWorld);

        void create(NewtonCollision* collision);
        void create(NewtonCollision* collision, float mass);

    private:
        const NewtonWorld*     m_newtonWorld;
        Vector                 m_origin;
        Vector                 m_inertia;
        float                  m_mass;
    };

    class Body
    {
        friend class Level;

    public:
        void prepare();
        void render(const Video* video);
        void setPositionAndRotation(const Vector& position,
                                    const Vector& rotation);
        Vector getPosition();

        NewtonBody*     m_newtonBody;        

    protected:

        Body(const XMLnode& node, const Game* game);

        const NewtonWorld*     m_newtonWorld;
        Matrix                 m_matrix;

        ~Body();

        void createNewtonBody(const NewtonCollision* newtonCollision,
                              const Vector& totalOrigin,
                              const Vector& position,
                              const Vector& rotation);

    private:

        float           m_totalMass;
        Vector          m_totalInertia;
        set<Collision*> m_collisions;

        void onSetForceAndTorque();
        static void onSetForceAndTorque(const NewtonBody* newtonBody);
    };

    class Level
    {
    public:
        Level(const Game* game);
        ~Level();
        void  load(const string& levelFile, StringSet& loaded = StringSet());
        void  render(const Video* video) const;
        void  prepare();
        Body* getBody(const string id);

        BodiesMap     m_bodies;
        CollisionsMap m_collisions;
        MaterialsMap  m_materials;
    
    private:
        const Game* m_game;
    };

}
#endif
