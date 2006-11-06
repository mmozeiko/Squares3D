#ifndef __INTRO_H__
#define __INTRO_H__

#include "common.h"
#include "state.h"
#include "system.h"
#include "vmath.h"
#include "video.h"
#include <Newton.h>

class Game;
class Texture;
class Mesh;

class Intro : public State
{
public:
    Intro();
    ~Intro();

    void control();
    void update(float delta);
    void updateStep(float delta);
    void prepare();
    void render() const;
    State::Type progress();

private:
    float m_timePassed;
    bool  m_nextState;
    bool  m_ballKicked;

    NewtonWorld* m_newtonWorld;

    vector<NewtonBody*> m_piece_body;
    vector<Matrix>      m_piece_matrix;

    NewtonBody*  m_ball_body;
    Matrix       m_ball_matrix;

    Texture*     m_logoTex;
    Texture*     m_ballTex;
    Texture*     m_bigLogoTex;
    unsigned int m_cubeListBase;

    Mesh*        m_mesh;

    void renderCube(int idx) const;
};

#endif
