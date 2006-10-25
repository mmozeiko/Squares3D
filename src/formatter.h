#ifndef __FORMATTER_H__
#define __FORMATTER_H__

#include "common.h"

class Formatter : public NoCopy
{
public:
    Formatter(const wstring& txt);

    template <typename T>
    inline Formatter& operator () (T value);

    inline Formatter& operator () (const string& value);

    operator wstring ();

private:
    wstring m_txt;

    void updateFirst(const wstring& value);
};

template <typename T>
Formatter& Formatter::operator () (T value)
{
    updateFirst(wcast<wstring>(value));
    return *this;
}

inline Formatter& Formatter::operator () (const string& value)
{
    updateFirst(wstring(value.begin(), value.end()));
    return *this;
}

#endif
