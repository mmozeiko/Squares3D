#ifndef __FORMATTER_H__
#define __FORMATTER_H__

#include "common.h"

template <typename T>
struct ConstRefForString
{
    typedef T Result;
};

template <>
struct ConstRefForString<string>
{
    typedef const string& Result;
};

class Formatter
{
public:
    Formatter(const wstring& txt);

    //template <typename T>
    //Formatter& operator () (typename ConstRefForString<T>::Result value);

    template <typename T>
    Formatter& operator () (T value);

    template <> // TODO: why this doesn't work ???
    Formatter& operator () (const string& value);

    operator wstring ();

private:
    wstring m_txt;

    void updateFirst(const wstring& value);
};

template <typename T>
//Formatter& Formatter::operator () (typename ConstRefForString<T>::Result value)
inline Formatter& Formatter::operator () (T value)
{
    updateFirst(wcast<wstring>(value));
    return *this;
}

template <>
inline Formatter& Formatter::operator () (const string& value)
{
    string x = value + "xxx";
    updateFirst(wstring(x.begin(), x.end()));
    return *this;
}

#endif
