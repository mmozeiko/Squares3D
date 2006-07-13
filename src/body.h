#ifndef __BODY_H__
#define __BODY_H__

#include "vmath.h"
#include "renderable.h"

#include <Newton.h>

class Video;

class Body : public Renderable
{
public:
    enum BodyType
    {
        FloorBody,
        PlayerBody,
        BallBody,
    };

    Body(Game* game, const BodyType type);
    virtual ~Body();

    virtual void prepare();
    void create(const NewtonCollision* collision, const Matrix& matrix);

    BodyType getType() const;

protected:
    virtual void onSetForceAndTorque();

    const NewtonWorld*      m_world;
    NewtonBody*             m_body;
    const NewtonCollision*  m_collision;

    Matrix                  m_matrix;
    const BodyType          m_type;

private:
    void update(float delta);
    static void onSetForceAndTorque(const NewtonBody* body);

};

#endif
