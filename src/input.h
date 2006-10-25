#ifndef __INPUT_H__
#define __INPUT_H__

#include <GL/glfw.h>
#include "common.h"
#include "system.h"

struct Mouse
{
    int x, y, z, b;
};

class Input : public System<Input>, public NoCopy
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

    void startCharBuffer();
    void endCharBuffer();
    int  popChar();

    void startButtonBuffer();
    void endButtonBuffer();
    int  popButton();

    // private methods, don't use it
    void addKey(int k);
    void addChar(int ch);
    void addButton(int b);

private:
    Mouse   m_mouse;
    IntList m_keyBuffer;
    IntList m_charBuffer;
    IntList m_buttonBuffer;
};

#endif
