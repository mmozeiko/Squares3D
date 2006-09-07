#include "messages.h"
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

Message::Message(const string &message, const Vector& initialPos, const char type) :
    m_message(message),
    m_position(initialPos),
    m_type(type)
{
}

Messages::Messages() :
    m_font("Arial_32pt_bold.bff")
{
    add(".", Vector(100,100,100), 1);
}


void Messages::applyFlowFunction(const float performance)
{
}

void Messages::update(const float performance)
{
}

void Messages::add(const string &message, const Vector& initialPos, const char type)
{
    Message msg(message, initialPos, type);
    m_buffer.push_back(msg);
}

void Messages::render()
{
    for each_(MsgBuffer, m_buffer, message)
    {
        Message msg = *message;
        m_font.begin();
        //x, y, z = gluProject(msg.m_position.x, msg.m_position.y, msg.m_position.z);
        //glTranslatef(x, y, 0);
        glTranslatef(55.0f, 55.0f, 0.0f);
        Vector v(1,0,1);
        glColor3fv(v.v);
        m_font.render(msg.m_message);
        m_font.end();

    }
}