#include "messages.h"
#include "video.h"
#include "message.h"

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

void Messages::remove(Message* message)
{
    MessageVector::iterator iter = m_buffer.begin();

    while (iter != m_buffer.end())
    {
        if (*iter == message)
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

    //correct positions to fit on screen
    if (vx < m_font->getWidth(message->getText()) / 2)
    {
        vx = m_font->getWidth(message->getText()) / 2;
    }
    else if (vx > Video::instance->getResolution().first - m_font->getWidth(message->getText()) / 2)
    {
        vx = Video::instance->getResolution().first - m_font->getWidth(message->getText()) / 2;
    }

    if (vy < 0)
    {
        vy = 0;
    }
    else if (vy > Video::instance->getResolution().second - m_font->getHeight() * 5)
    {
        vy = Video::instance->getResolution().second - m_font->getHeight() * 5;
    }

    message->m_position = Vector(static_cast<float>(vx),
                                 static_cast<float>(vy),
                                 static_cast<float>(vz));
    m_buffer.push_back(message);
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
            
            glColor4fv(message->m_color.v);
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
