#ifndef __FORMATTER_H__
#define __FORMATTER_H__

#include "common.h"

class Formatter : public NoCopy
{
public:
    Formatter(const string& txt);

    template <typename T>
    inline Formatter& operator () (T value);

    operator string ();

private:
    string m_txt;

    void updateFirst(const string& value);
};

template <typename T>
Formatter& Formatter::operator () (T value)
{
    updateFirst(cast<string>(value));
    return *this;
}

#endif
