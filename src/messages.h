#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "common.h"
#include "font.h"
#include "vmath.h"

struct Message
{
    enum MessageType
    {
        Type_Unknown,
        Type_Flowing,
        Type_Static,
    };

    Message(const string &message, const Vector& position, const Vector& color, MessageType type = Type_Unknown);
    
    string      m_text;
    Vector      m_position;
    Vector      m_color;
    MessageType m_type;
    float       m_timeToLive;
};

typedef list<Message> MessageVector;

class Messages : NoCopy
{
public:
    Messages();
    void update(float delta);
    void add(const Message& message);
    void render() const;

private:
    const Font    m_font;
    MessageVector m_buffer;
};


#endif
