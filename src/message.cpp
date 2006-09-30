#include "message.h"
#include "messages.h"
#include "formatter.h"
#include "font.h"
#include "video.h"

Message::Message(const wstring&        message, 
                 const Vector&         position, 
                 const Vector&         color, 
                 const Font::AlignType align,
                 const int             fontSize) :
    m_text(message),
    m_position(position),
    m_color(color),
    m_align(align),
    m_fontSize(fontSize)
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

void Message::render(const Font* font) const
{
    if (m_position.z <= 1.0)
    {
        glPushMatrix();
        glTranslatef(m_position.x, m_position.y, m_position.z);
        glColor4fv(m_color.v);
        font->render(getText(), m_align);
        glPopMatrix();
    }
}

BlinkingMessage::BlinkingMessage(
                const wstring&        message, 
                const Vector&         position, 
                const Vector&         color, 
                const Font::AlignType align,
                const int             fontSize,
                const float           blinkIntensity) :
    Message(message, position, color, align, fontSize),
    m_visible(true),
    m_blinkCurrent(blinkIntensity),
    m_blinkIntensity(blinkIntensity)
{
}

void BlinkingMessage::render(const Font* font) const
{
    if (m_visible)   
    {
        Message::render(font);
    }
}

bool BlinkingMessage::applyDelta(float delta)
{
    if (m_visible)
    {
        m_blinkCurrent -= delta;
        if (m_blinkCurrent < 0.0f)
        {
            m_visible = false;
        }
    }
    else
    {
        m_blinkCurrent += delta;
        if (m_blinkCurrent > m_blinkIntensity)
        {
            m_visible = true;
        }
    }
    return false;
}

FlowingMessage::FlowingMessage(
                    const wstring&        message, 
                    const Vector&         position, 
                    const Vector&         color, 
                    const Font::AlignType align,
                    const float           timeToLive) : 
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

    //message fading
    if (m_timeToLive < 0.5f)
    {
        m_color.w = 2.0f*m_timeToLive;
    }

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

