#ifndef __FILE_H__
#define __FILE_H__

#include <istream>
#include <ostream>
#include "common.h"

namespace File
{
    void init(const char* argv0);
    void done();
    bool exists(const string& filename);

    class InputBuffer;
    class OutputBuffer;

    class Reader : public std::istream
    {
    public:
        Reader(const string& filename = "");
        ~Reader();

        bool is_open() const;
        bool open(const string& filename);
        void close();

        unsigned int filesize() const;

    private:
        InputBuffer* m_buf;
    };

    class Writer : public std::ostream
    {
    public:
        Writer(const string& filename = "", bool append = false);
        ~Writer();

        bool is_open() const;
        bool open(const string& filename, bool append = false);
        void close();

    private:
        OutputBuffer* m_buf;
    };

};

#endif
