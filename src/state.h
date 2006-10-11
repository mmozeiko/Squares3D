#ifndef __STATE_H__
#define __STATE_H__

#include "common.h"

class State : NoCopy
{
public:

    enum Type
    {
        Current,

        Intro,
        MenuEasy,
        MenuNormal,
        MenuHard,
        Lobby,
        World,
        WorldEasy,
        WorldNormal,
        WorldHard,
        Quit,
    };

    Type m_current;

    State() : m_current(Current) {}
    virtual ~State() {}

    virtual void init() {}
    virtual void control() = 0;
    virtual void update(float delta) = 0;
    virtual void updateStep(float delta) = 0;
    virtual void prepare() = 0;
    virtual void render() const = 0;
    virtual State::Type progress() = 0;

};

#endif
