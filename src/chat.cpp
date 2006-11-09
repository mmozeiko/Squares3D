#include "chat.h"
#include "video.h"
#include "input.h"
#include "font.h"
#include "network.h"

Chat::Chat(const string& player, const Vector& color) :
    m_font(NULL),
    m_player(player),
    m_color(color),
    m_count(0),
    m_active(false)
{
    m_font = Font::get(Video::instance->getResolution().first >= 1024 ? "Arial_32pt_bold" : "Arial_16pt_bold");
}

bool Chat::updateKey(int key)
{
    bool ret = false;

    const size_t m = m_font->getWidth("M")* (15 + Network::instance->getMaxPlayerName().size());

    int ch = Input::instance->popChar();
    while (ch != -1)
    {
        if (ch >= 32 && ch < 128 && m_font->hasChar(ch))
        {
            if (m_active==false)
            {
                if (ch != ' ')
                {
                    m_message += ch;
                    m_active = true;
                    ret = true;
                    m_removeTimer.reset();
                }
            }
            else if (m_font->getWidth(m_message + string(1, char(ch))) < static_cast<int>(m))
            {
                m_message.push_back(ch);
                ret = true;
                m_removeTimer.reset();
            }
        }
        ch = Input::instance->popChar();
    }

    if (m_active && (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER))
    {
        Network::instance->addChatPacket(m_message);
        recieve(m_player, m_color, m_message);
        m_message = "";
        m_active = false;
        ret = true;
    }
    else if (m_active && (key == GLFW_KEY_ESC))
    {
        m_active = false;
        m_message = "";
        return true;
    }

    if (m_active && Input::instance->key(GLFW_KEY_BACKSPACE) && m_backTimer.read() > 0.05f && m_message.empty()==false)
    {
        m_message.erase(m_message.end()-1); // TODO: UTF-8 warning!!
        m_backTimer.reset();
    }

    if (m_removeTimer.read() > 3.0f)
    {
        for (int i=0; i<m_count-1; i++)
        {
            m_players[i] = m_players[i+1];
            m_colors[i] = m_colors[i+1];
            m_messages[i] = m_messages[i+1];
        }
        if (m_count > 0)
        {
            m_count--;
        }
        m_removeTimer.reset();
    }

    return ret;
}

void Chat::render() const
{
    float x = static_cast<float>(Video::instance->getResolution().first);
    float y = static_cast<float>(Video::instance->getResolution().second);

    const size_t m = m_font->getWidth("M")* (17 + 2 + Network::instance->getMaxPlayerName().size());
    float yy = y - m_font->getHeight();
    float xx = (x - m)/2.0f;

    if (Video::instance->getResolution().first >= 1024)
    {
        m_font->begin();
    }
    else
    {
        m_font->begin(true, Vector::One*0.1f, 0.5f);
    }

    if (m_active || m_count > 0)
    {
        glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
        Video::instance->renderRoundRect(
            Vector(xx, yy - (m_count-1 + (m_active ? 1 : 0)) * (m_font->getHeight()), 0),
            Vector((x + m)/2.0f, y, 0),
            m_font->getHeight()/3.0f
        );
    }

    for (int i=0; i<m_count; i++)
    {
        // render
        glPushMatrix();
        glTranslatef(xx, yy, 0.0f);
        glColor3fv(m_colors[i].v);
        m_font->render(m_players[i] + ": " + m_messages[i]);
        glPopMatrix();
        
        yy -= m_font->getHeight();
    }

    // render current
    if (m_active)
    {
        string render = m_player + ": " + m_message;
        if (fmod(m_timer.read(), 1.0f) > 0.5f)
        {
            render.push_back('_');
        }
        glPushMatrix();
        glTranslatef(xx, yy, 0.0f);
        glColor3fv(m_color.v);
        m_font->render(render);
        glPopMatrix();
    }

    m_font->end();
}

void Chat::recieve(const string& player, const Vector& color, const string& message)
{
    if (m_count == CHAT_MSG_COUNT)
    {
        for (int i=0; i<CHAT_MSG_COUNT-1; i++)
        {
            m_players[i] = m_players[i+1];
            m_colors[i] = m_colors[i+1];
            m_messages[i] = m_messages[i+1];
        }
    }
    else
    {
        m_count++;
    }

    m_players[m_count-1] = player;
    m_colors[m_count-1] = color;
    m_messages[m_count-1] = message;

    m_removeTimer.reset();
}
