#ifndef __BODY_H__
#define __BODY_H__

#include "vmath.h"
#include "renderable.h"

#include <Newton.h>

class Video;

class Body : public Renderable
{
public:

    Body(Game* game);
    virtual ~Body();

    virtual void prepare();
    void create(const NewtonCollision* collision, const Matrix& matrix);

protected:
    virtual void onSetForceAndTorque();

    const NewtonWorld*      m_world;
    NewtonBody*             m_body;
    const NewtonCollision*  m_collision;

    Matrix                  m_matrix;

private:
    void update(float delta);
    static void onSetForceAndTorque(const NewtonBody* body);

};

#endif
