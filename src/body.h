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

    void Create(const NewtonCollision* collision, const Matrix& matrix);

    virtual void Prepare();
    virtual void onRender(const Video* video) const = 0;
    
    void Render(const Video* video) const;

    BodyType GetType() const;

protected:
    virtual void onSetForceAndTorque();

    const NewtonWorld*      _world;
    NewtonBody*             _body;
    const NewtonCollision*  _collision;

    Matrix                  _matrix;
    const BodyType          _type;

private:
    static void onSetForceAndTorque(const NewtonBody* body);

};

#endif
