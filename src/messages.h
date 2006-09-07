#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "common.h"
#include "video.h"
#include "font.h"
#include "vmath.h"

struct Message
{
    Message(const string &message, const Vector& initialPos, const char type);
    string m_message;
    Vector m_position;
    char   m_type;
};

typedef std::vector<Message> MsgBuffer;

class Messages
{
public:
    Messages();
    void update(const float performance);
    void add(const string &message, const Vector& initialPos, const char type);
    void render();

    const Font  m_font;

private:
    void applyFlowFunction(const float performance);
    MsgBuffer   m_buffer;
};


#endif

