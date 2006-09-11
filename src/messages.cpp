#include "messages.h"
#include "video.h"

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

Messages::Messages() : m_font(Font::get("Arial_32pt_bold"))
{
}

void Messages::update(float delta)
{
    MessageVector::iterator iter = m_buffer.begin();

    while (iter != m_buffer.end())
    {
        Message* message = *iter;

        message->applyFlow(delta);

        if (message->applyDelta(delta))
        {
            delete *iter;
            iter = m_buffer.erase(iter);
        }
        else
        {
            iter++;
        }
    }
}

void Messages::add3D(Message* message)
{
    Matrix modelview, projection;
    int viewport[4];
    
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview.m);
    glGetFloatv(GL_PROJECTION_MATRIX, projection.m);
    glGetIntegerv(GL_VIEWPORT, viewport);
    double m[16], p[16];
    std::copy(&modelview.m[0], &modelview.m[16], m);
    std::copy(&projection.m[0], &projection.m[16], p);
 
    double vx, vy, vz;
    gluProject(
        message->m_position.x,
        message->m_position.y,
        message->m_position.z,
        m, p, viewport,
        &vx, &vy, &vz);

    m_buffer.push_back(message);
    m_buffer.back()->m_position = Vector(static_cast<float>(vx),
                                         static_cast<float>(vy),
                                         static_cast<float>(vz));
}

void Messages::add2D(Message* message)
{
    m_buffer.push_back(message);
}

void Messages::render() const
{
    m_font->begin();

    for each_const(MessageVector, m_buffer, iter)
    {
        Message* message = *iter;
        if (message->m_position.z <= 1.0)
        {
            glPushMatrix();
            glTranslatef(
                        message->m_position.x,
                        message->m_position.y,
                        message->m_position.z);
            glColor3fv(message->m_color.v);
            m_font->render(message->getText(), message->m_align);
            glPopMatrix();
        }
    }
    m_font->end();
}

Messages::~Messages()
{
    for each_const(MessageVector, m_buffer, iter)
    {
        delete *iter;
    }
}
