#include "formatter.h"

Formatter::Formatter(const wstring& txt) : m_txt(txt)
{
}
   
Formatter::operator wstring ()
{
    return m_txt;
}

void Formatter::updateFirst(const wstring& value)
{
    size_t offset = 0;
    size_t pos = m_txt.find(L"$");
    while (pos != wstring::npos)
    {
        if (pos == m_txt.size()-1)
        {
            break;
        }

        if (m_txt[pos+1] == L'$')
        {
            offset = pos+2;
        }
        else
        {
            int num = m_txt[pos+1] - L'0';
            if (num == 1)
            {
                m_txt = m_txt.substr(0, pos) + value + m_txt.substr(pos+2, m_txt.size());
                offset = pos + value.size();
            }
            else
            {
                m_txt[pos+1]--;
                offset = pos+2;
            }
        }

        pos = m_txt.find(L"$", offset);
    }
}
