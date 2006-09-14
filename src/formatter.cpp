#include "formatter.h"

Formatter::Formatter(const wstring& txt) : m_txt(txt)
{
}

Formatter& Formatter::operator () (const string& value)
{
    size_t pos = m_txt.find(L"%s");
    if (pos != wstring::npos)
    {
        m_txt = m_txt.substr(0, pos) + wcast<wstring>(value) + m_txt.substr(pos+2, m_txt.size());
    }
    return *this;
}

Formatter& Formatter::operator () (float value)
{
    size_t pos = m_txt.find(L"%f");
    if (pos != wstring::npos)
    {
        m_txt = m_txt.substr(0, pos) + wcast<wstring>(value) + m_txt.substr(pos+2, m_txt.size());
    }
    return *this;
}

Formatter& Formatter::operator () (int value)
{
    size_t pos = m_txt.find(L"%i");
    if (pos != wstring::npos)
    {
        m_txt = m_txt.substr(0, pos) + wcast<wstring>(value) + m_txt.substr(pos+2, m_txt.size());
    }
    return *this;
}

Formatter::operator wstring ()
{
    return m_txt;
}
