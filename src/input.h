#ifndef __INPUT_H__
#define __INPUT_H__

#include <GL/glfw.h>
#include "common.h"
#include "system.h"

struct Mouse
{
    int x, y, z, b;
};

class Input : public System<Input>, NoCopy
{
public:
    Input();
    ~Input();

    void update();

    bool key(int key) const;
    const Mouse& mouse() const;

    void             startKeyBuffer();
    const IntVector& getKeyBuffer() const;
    void             endKeyBuffer();
    int              popKey();

    // private method, don't use it
    void          addKey(int c);

private:
    Mouse     m_mouse;
    IntVector m_keyBuffer;
};

#endif
