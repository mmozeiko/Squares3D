#include "messages.h"
#include "video.h"

//#include <GLU.h>

    //addend = self.msgFlowFunction(fault[2], px)
    //text = fault[0]
    //pts = fault[1]
    //x, y = W / 2 - self.font_aa.getSize(text)[0] * scale / 2, H - 50 * scale
    //y = y + fault[2]
      
  //  glColor4f(1.0, 0.8, 0.0, addend)
  //  self.font_aa.begin()
  //  self.font_aa.renderShadowed(text, x, y, (0,0,0), scale)
  //  if pts:
  //    glColor4f(0.5, 0.8, 0.1, addend)
  //    pts = '+' + pts
  //    x = W / 2 - self.font_aa.getSize(pts)[0] * scale / 2
  //    y -= self.font_aa.getSize(pts)[1] * scale
  //    self.font_aa.renderShadowed(pts, x, y, (0,0,0), scale)
  //  self.font_aa.end()
  //  fault[2] += 200 * (1.2 - addend) * performance
  //else:
  //  self.faultMsgList = self.faultMsgList[1:]

Message::Message(const string &message, 
                 const Vector& position, 
                 const Vector& color, 
                 MessageType type) :
    m_text(message),
    m_position(position),
    m_color(color),
    m_type(type),
    m_timeToLive(1)
{
    switch (m_type)
    {
    case Type_Flowing: m_timeToLive = 2;
                       break;
    //......
    default: m_timeToLive = 3;
    }
}

Messages::Messages() : m_font("Arial_32pt_bold.bff")
{
    //add(Message("X", 4 * Vector::X, Vector(1, 0, 0)));
    //add(Message("Y", 4 * Vector::Y, Vector(0, 1, 0)));
    //add(Message("Z", 4 * Vector::Z, Vector(0, 0, 1)));
    add(Message("PEETERIS JAU NAV ATBRAUCIS SHEIT SUNJUS POTEET..", Vector(0, 0, 0), Vector(1, 1, 1)));
}


void _applyFlowFunction(float delta, Message* message)
{
    message->m_position.y += delta;
}

void Messages::update(float delta)
{
    MessageVector::iterator iter = m_buffer.begin();

    while (iter != m_buffer.end())
    {
        Message& message = *iter;
        _applyFlowFunction(delta, &message);
        message.m_timeToLive -= delta;

        if (message.m_timeToLive <= 0.0f)
        {
            iter = m_buffer.erase(iter);
        }
        else
        {
            iter++;
        }
    }
}

void Messages::add(const Message& message)
{
    m_buffer.push_back(message);
}

void Messages::render() const
{
    Matrix modelview, projection;
    int viewport[4];
    
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview.m);
    glGetFloatv(GL_PROJECTION_MATRIX, projection.m);
    glGetIntegerv(GL_VIEWPORT, viewport);
    double m[16], p[16];
    std::copy(&modelview.m[0], &modelview.m[16], m);
    std::copy(&projection.m[0], &projection.m[16], p);

    m_font.begin(Font::Align_Center);

    for each_const(MessageVector, m_buffer, iter)
    {
        const Message& message = *iter;
        
        double vx, vy, vz;
        gluProject(
            message.m_position.x,
            message.m_position.y,
            message.m_position.z,
            m, p, viewport,
            &vx, &vy, &vz);

        if (vz <= 1.0)
        {
            glPushMatrix();
            glTranslated(vx, vy, 0.0);
            glColor3fv(message.m_color.v);
            m_font.render(message.m_text);
            glPopMatrix();
        }
    }
    m_font.end();
}
