#ifndef __CHAT_H__
#define __CHAT_H__

#include "common.h"
#include "timer.h"
#include "vmath.h"

class Font;

static const int CHAT_MSG_COUNT = 4;

class Chat
{
public:
    Chat(const string& player, const Vector& color);

    bool updateKey(int key);
    void render() const;
    void recieve(const string& player, const Vector& color, const string& message);

private:
    const Font* m_font;

    const string& m_player;
    const Vector& m_color;
    string m_message;
    
    string m_players[CHAT_MSG_COUNT];
    Vector m_colors[CHAT_MSG_COUNT];
    string m_messages[CHAT_MSG_COUNT];

    int m_count;

    bool m_active;

    Timer m_timer;
    Timer m_backTimer;
    Timer m_removeTimer;
};

#endif
