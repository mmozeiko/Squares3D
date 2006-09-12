#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "common.h"
#include "vmath.h"
#include "font.h"

class Messages;
class Message;

typedef list<Message*> MessageVector;

class Messages : NoCopy
{
public:
    Messages();
    ~Messages();
    void update(float delta);
    void add2D(Message* message);
    void add3D(Message* message);
    void render() const;

private:
    const Font*   m_font;
    MessageVector m_buffer;
};


#endif
