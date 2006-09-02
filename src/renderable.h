#ifndef __RENDERABLE_H__
#define __RENDERABLE_H__

#include <Newton.h>
#include "common.h"

class Game;
class Input;
class Video;

class Renderable
{
public:
    Renderable(Game* game);

    virtual void control(const Input* input) = 0;
    virtual void update(float delta) = 0;
    virtual void prepare() = 0;
    virtual void render(const Video* video) const = 0;
    
protected:
    Game* m_game;
};

#endif
