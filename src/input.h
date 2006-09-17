#ifndef __INPUT_H__
#define __INPUT_H__

#include <GL/glfw.h>
#include "common.h"
#include "system.h"

struct Mouse
{
    int x, y, z;
};

class Input : public System<Input>, NoCopy
{
public:
    Input();
    ~Input();

    void update();

    bool  key(int key) const;
    const Mouse& mouse() const;

    void startKeyBuffer();
    void endKeyBuffer();
    int  popKey();

    void startButtonBuffer();
    void endButtonBuffer();
    int  popButton();

    // private methods, don't use it
    void addKey(int k);
    void addButton(int b);

private:
    Mouse   m_mouse;
    IntList m_keyBuffer;
    IntList m_buttonBuffer;
};

#endif
