#ifndef __FORMATTER_H__
#define __FORMATTER_H__

#include "common.h"

class Formatter
{
public:
    Formatter(const wstring& txt);

    Formatter& operator () (const string& value);
    Formatter& operator () (float value);
    Formatter& operator () (int value);
    operator wstring ();

private:
    wstring m_txt;
};

#endif
