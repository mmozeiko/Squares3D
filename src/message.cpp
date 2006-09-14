#include "message.h"
#include "messages.h"
#include "formatter.h"

Message::Message(const wstring&        message, 
                 const Vector&         position, 
                 const Vector&         color, 
                 const Font::AlignType align) :
    m_text(message),
    m_position(position),
    m_color(color),
    m_align(align)
{
}

wstring Message::getText() const
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
                    const wstring&        message, 
                    const Vector&         position, 
                    const Vector&         color, 
                    const Font::AlignType align,
                    const float              timeToLive) : 
    Message(message, position, color, align),
    m_timeToLive(timeToLive)
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

ScoreMessage::ScoreMessage(const wstring&        message, 
                           const Vector&         position, 
                           const Vector&         color,
                           const int             score,
                           const Font::AlignType align) : 
    Message(message, position, color, align),
    m_score(score)
{
}

wstring ScoreMessage::getText() const
{
    return Formatter(m_text)(m_score);
}

ComboMessage::ComboMessage(const wstring&        message, 
                           const Vector&         position, 
                           const Vector&         color,
                           const int             points,
                           const Font::AlignType align) : 
    Message(message, position, color, align),
    m_points(points)
{
}

wstring ComboMessage::getText() const
{
    if (m_points > 1)
    {
        return Formatter(m_text)(m_points);
    }
    return L"";
}

