#include "message.h"
#include "messages.h"
#include "formatter.h"
#include "font.h"
#include "video.h"
#include "colors.h"

static const float FADE_SPEED = 2.0f;

Message::Message(const string&         message, 
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
        const string&   message, 
        const Vector&   position, 
        const Vector&   color, 
        Font::AlignType align,
        int             fontSize,
        float           blinkIntensity) :
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
                    const string&   message, 
                    const Vector&   position, 
                    const Vector&   color, 
                    Font::AlignType align,
                    float           timeToLive) : 
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
        m_color.w = 2.0f * m_timeToLive;
    }

    return m_timeToLive <= 0.0f;
}

ScoreMessage::ScoreMessage(const string&   message, 
                           const Vector&   position, 
                           const Vector&   color,
                           int             score,
                           Font::AlignType align) : 
    Message(message, position, color, align),
    m_score(score)
{
}

string ScoreMessage::getText() const
{
    return Formatter(m_text)(m_score);
}

ComboMessage::ComboMessage(const string&   message, 
                           const Vector&   position, 
                           const Vector&   color,
                           int             points,
                           Font::AlignType align,
                           int             fontSize) : 
    Message(message, position, color, align, fontSize),
    m_points(points),
    m_previousPoints(points)
{
}

string ComboMessage::getText() const
{
    return Formatter(m_text)(m_previousPoints);
}

bool ComboMessage::applyDelta(float delta)
{
    if (m_points > 1)
    {
        m_color.w = 1.0f;
        m_previousPoints = m_points;
    }
    else
    {
        if (m_color.w > 0.0f)
        {
            m_color.w -= delta * FADE_SPEED;
        }
    }
    return false;
}

void ComboMessage::render(const Font* font) const
{
    if (m_color.w > 0.0f)
    {
        Message::render(font);
    }
}

LastTouchedMessage::LastTouchedMessage(const Vector&   position, 
                                       Font::AlignType align,
                                       int             fontSize) : 
    Message("", position, White, align, fontSize),
    m_fadeOut(false)
{
}

bool LastTouchedMessage::applyDelta(float delta)
{
    if (m_fadeOut)
    {
        if (m_color.w > 0.0f)
        {
            m_color.w -= delta * FADE_SPEED;
        }
    }
    else
    {
        m_color.w = 1.0f;
    }
    return false;
}

void LastTouchedMessage::setMessage(const string& message, const Vector& color)
{
    m_text = message;
    m_color = color;
    m_fadeOut = false;
}

void LastTouchedMessage::render(const Font* font) const
{
    if (m_color.w > 0.0f)
    {
        Message::render(font);
    }
}

void LastTouchedMessage::fadeOut()
{
    m_fadeOut = true;
}