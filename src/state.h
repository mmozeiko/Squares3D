#ifndef __STATE_H__
#define __STATE_H__

#include <Newton.h>
#include "common.h"

class Game;
class Input;

class State
{
public:
	State(Game* game);

    virtual void control(const Input* input, float delta) = 0;
    virtual void update() = 0;
    virtual void prepare() = 0;
    virtual void render() const = 0;
    
protected:
	Game* m_game;
};

#endif
