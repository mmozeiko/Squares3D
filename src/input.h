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
    const Mouse& mouse(int id = 0) const;

    void          startKeyBuffer();
    const string& getKeyBuffer() const;
    string&       getKeyBuffer();
    void          endKeyBuffer();

    // private method, don't use it
    void    addChar(char c);

private:
    int           m_mouseCount;
    bool          m_manymouses;
    vector<Mouse> m_mouses;
    string        m_keyBuffer;
};

#endif
