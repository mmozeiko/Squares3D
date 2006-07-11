#ifndef __INPUT_H__
#define __INPUT_H__

#include "common.h"
#include <GL/glfw.h>

struct Mouse
{
    int x;
    int y;
    int z;
    int b;
};

class Input
{
public:
    Input();
    ~Input();

    void process();

    bool key(int key) const;
    const Mouse& mouse(int id = 0) const;

    void    startKeyBuffer();
    string  getKeyBuffer() const;
    void    endKeyBuffer();

private:
    int           m_mouseCount;
    bool          m_manymouses;
    vector<Mouse> m_mouses;
    string        m_keyBuffer;
};


#endif
