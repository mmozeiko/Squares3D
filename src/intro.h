#ifndef __INTRO_H__
#define __INTRO_H__

#include "common.h"
#include "state.h"
#include "system.h"
#include "vmath.h"

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
    State::Type progress() const;

private:
    float m_timePassed;
    bool m_nextState;

};

#endif
