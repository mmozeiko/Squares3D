#ifndef __BODY_H__
#define __BODY_H__

#include <Newton.h>
#include "vmath.h"
#include "video.h"

const Vector gravityVec(0.0f, -9.81f, 0.0f);

class Body
{
public:
    Body(const NewtonWorld* world);
    virtual ~Body();

    void Create(const NewtonCollision* collision, const Matrix& matrix);

    virtual void Prepare();
    virtual void onRender(const Video& video) const = 0;
    
    void Render(const Video& video) const;

protected:
    virtual void onSetForceAndTorque();

    const NewtonWorld*      _world;
    NewtonBody*             _body;
    const NewtonCollision*  _collision;

    Matrix                  _matrix;

private:
    static void onSetForceAndTorque(const NewtonBody* body);

};

#endif
