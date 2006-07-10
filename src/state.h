#ifndef __STATE_H__
#define __STATE_H__

#include <Newton.h>
#include "common.h"

class Game;

class State
{
public:
	State(Game* game);

    virtual void Control(float delta) = 0;
    virtual void Update() = 0;
    virtual void Prepare() = 0;
    virtual void Render() const = 0;
    
protected:
	Game* _game;
};
#endif