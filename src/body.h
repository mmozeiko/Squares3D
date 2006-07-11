#ifndef __BODY_H__
#define __BODY_H__

#include "vmath.h"
#include <Newton.h>

const Vector gravityVec(0.0f, -9.81f, 0.0f);

class Video;

class Body
{
public:
    enum BodyType
    {
        FloorBody,
        PlayerBody,
        BallBody,
    };

    Body(const NewtonWorld* world, const BodyType type);
    virtual ~Body();

    void create(const NewtonCollision* collision, const Matrix& matrix);

    virtual void prepare();
    virtual void onRender(const Video* video) const = 0;
    
    void render(const Video* video) const;

    BodyType getType() const;

protected:
    virtual void onSetForceAndTorque();

    const NewtonWorld*      m_world;
    NewtonBody*             m_body;
    const NewtonCollision*  m_collision;

    Matrix                  m_matrix;
    const BodyType          m_type;

private:
    static void onSetForceAndTorque(const NewtonBody* body);

};

#endif
