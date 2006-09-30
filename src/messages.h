#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "common.h"
#include "vmath.h"
#include "font.h"

class Messages;
class Message;

typedef list<Message*> MessageVector;
typedef map<int, MessageVector> MessageVectorsByHeight;
typedef map<int, const Font*> Fonts;

class Messages : NoCopy
{
public:
    Messages();
    ~Messages();
    void update(float delta);
    void add2D(Message* message);
    void add3D(Message* message);
    void remove(Message* message);
    void render() const;

    Fonts m_fonts;
private:
    MessageVectorsByHeight m_buffer;
};


#endif
