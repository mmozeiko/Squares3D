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
        const string&         message, 
        const Vector&         position, 
        const Vector&         color, 
        const Font::AlignType align = Font::Align_Left);
    
protected:

    virtual string getText() const;
    
    virtual void applyFlow(float delta);

    virtual bool applyDelta(float delta);

    string          m_text;
    Vector          m_position;
    Vector          m_color;
    Font::AlignType m_align;
};

class FlowingMessage : public Message
{
public:
    FlowingMessage(
        const string&         message, 
        const Vector&         position, 
        const Vector&         color, 
        const Font::AlignType align = Font::Align_Left);

protected:

    void applyFlow(float delta);

    bool applyDelta(float delta);

    float m_timeToLive;
};


class ScoreMessage : public Message
{
public:
    ScoreMessage(
        const string&         message, 
        const Vector&         position, 
        const Vector&         color,
        const int             score,
        const Font::AlignType align = Font::Align_Left);

    int m_score;

protected:

    string getText() const;
};

class ComboMessage : public Message
{
public:
    ComboMessage(
        const string&         message, 
        const Vector&         position, 
        const Vector&         color,
        const int             points,
        const Font::AlignType align = Font::Align_Center);
    
    int m_points;
protected:
    string getText() const;
};

#endif
