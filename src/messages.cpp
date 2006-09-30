#include "messages.h"
#include "video.h"
#include "message.h"

Messages::Messages()
{
    m_fonts[32] = Font::get("Arial_32pt_bold");
    m_fonts[72] = Font::get("Arial_72pt_bold");
}

void Messages::update(float delta)
{
    for each_(MessageVectorsByHeight, m_buffer, iter)
    {
        MessageVector* currentHeightbuffer = &iter->second;
        MessageVector::iterator iter2 = currentHeightbuffer->begin();

        while (iter2 != currentHeightbuffer->end())
        {
            Message* message = *iter2;

            message->applyFlow(delta);

            if (message->applyDelta(delta))
            {
                delete message;
                iter2 = currentHeightbuffer->erase(iter2);
            }
            else
            {
                iter2++;
            }
        }
    }
}

void Messages::remove(Message* message)
{
    MessageVector* currentHeightbuffer = &m_buffer.find(message->m_fontSize)->second;
    MessageVector::iterator iter = currentHeightbuffer->begin();

    while (iter != currentHeightbuffer->end())
    {
        if (*iter == message)
        {
            delete *iter;
            iter = currentHeightbuffer->erase(iter);
        }
        else
        {
            iter++;
        }
    }
}

void Messages::add3D(Message* message)
{
    const Font* font = m_fonts.find(message->m_fontSize)->second;

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
    if (vx < font->getWidth(message->getText()) / 2)
    {
        vx = font->getWidth(message->getText()) / 2;
    }
    else if (vx > Video::instance->getResolution().first - font->getWidth(message->getText()) / 2)
    {
        vx = Video::instance->getResolution().first - font->getWidth(message->getText()) / 2;
    }

    if (vy < 0)
    {
        vy = 0;
    }
    else if (vy > Video::instance->getResolution().second - font->getHeight() * 5)
    {
        vy = Video::instance->getResolution().second - font->getHeight() * 5;
    }

    message->m_position = Vector(static_cast<float>(vx),
                                 static_cast<float>(vy),
                                 static_cast<float>(vz));

    m_buffer[message->getFontSize()].push_back(message);
}

void Messages::add2D(Message* message)
{
    m_buffer[message->getFontSize()].push_back(message);
}

void Messages::render() const
{
    for each_const(MessageVectorsByHeight, m_buffer, iter)
    {
        const Font* font = m_fonts.find(iter->first)->second;
        font->begin();
        for each_const(MessageVector, iter->second, iter2)
        {
            Message* message = *iter2;
            message->render(font);
        }
        font->end();
    }
}

Messages::~Messages()
{
    for each_const(MessageVectorsByHeight, m_buffer, iter)
    {
        MessageVector currentHeightbuffer = iter->second;
        for each_const(MessageVector, currentHeightbuffer, iter2)
        {
            delete *iter2;    
        }
    }
}
