#include "message.h"
#include "messages.h"

Message::Message(const string          &message, 
                 const Vector&         position, 
                 const Vector&         color, 
                 const Font::AlignType align) :
    m_text(message),
    m_position(position),
    m_color(color),
    m_align(align)
{
}

string Message::getText() const
{
    return m_text;
}

void Message::applyFlow(float delta)
{
}

bool Message::applyDelta(float delta)
{
    return false;
}
    
FlowingMessage::FlowingMessage(
                    const string&         message, 
                    const Vector&         position, 
                    const Vector&         color, 
                    const Font::AlignType align) : 
    Message(message, position, color, align),
    m_timeToLive(2)
{
}

void FlowingMessage::applyFlow(float delta)
{
    m_position.y += 50.0f * delta;
}

bool FlowingMessage::applyDelta(float delta)
{
    m_timeToLive -= delta;
    return m_timeToLive <= 0.0f;
}

ScoreMessage::ScoreMessage(const string&         message, 
                           const Vector&         position, 
                           const Vector&         color,
                           const int             score,
                           const Font::AlignType align) : 
    Message(message, position, color, align),
    m_score(score)
{
}

string ScoreMessage::getText() const
{
    return m_text + ": " + cast<string>(m_score);
}

ComboMessage::ComboMessage(const string&         message, 
                           const Vector&         position, 
                           const Vector&         color,
                           const int             points,
                           const Font::AlignType align) : 
    Message(message, position, color, align),
    m_points(points)
{
}

string ComboMessage::getText() const
{
    string returnText = "";
    if (m_points > 1)
    {
        returnText = cast<string>(m_points) + m_text;
    }
    return returnText;
}
