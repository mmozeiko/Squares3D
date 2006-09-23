#ifndef __MESSAGE__H__
#define __MESSAGE__H__

#include "common.h"
#include "font.h"
#include "vmath.h"

class Messages;

class Message
{
    friend class Messages;
public:
    Message(
        const wstring&        message, 
        const Vector&         position, 
        const Vector&         color, 
        const Font::AlignType align = Font::Align_Left);
    
protected:

    virtual wstring getText() const;
    
    virtual void applyFlow(float delta);

    virtual bool applyDelta(float delta);

    wstring         m_text;
    Vector          m_position;
    Vector          m_color;
    Font::AlignType m_align;
};

class FlowingMessage : public Message
{
public:
    FlowingMessage(
        const wstring&        message, 
        const Vector&         position, 
        const Vector&         color, 
        const Font::AlignType align = Font::Align_Left,
        const float              timeToLive = 3);

protected:

    void applyFlow(float delta);

    bool applyDelta(float delta);

    float m_timeToLive;
};


class ScoreMessage : public Message
{
public:
    ScoreMessage(
        const wstring&        message, 
        const Vector&         position, 
        const Vector&         color,
        const int             score,
        const Font::AlignType align = Font::Align_Left);

    int m_score;

protected:

    wstring getText() const;
};

class ComboMessage : public Message
{
public:
    ComboMessage(
        const wstring&        message, 
        const Vector&         position, 
        const Vector&         color,
        const int             points,
        const Font::AlignType align = Font::Align_Center);
    
    int m_points;

protected:
    wstring getText() const;
};

#endif
