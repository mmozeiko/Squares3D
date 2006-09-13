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
        Menu,
        Lobby,
        World,
    };
    virtual ~State() {}

    virtual void control() = 0;
    virtual void update(float delta) = 0;
    virtual void updateStep(float delta) = 0;
    virtual void prepare() = 0;
    virtual void render() const = 0;
    virtual Type progress() const = 0;
    //virtual StateType getNextState() const;
};

#endif
