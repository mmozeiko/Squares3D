#ifndef __FILE_H__
#define __FILE_H__

#include <istream>
#include <ostream>
#include <string>

namespace File
{
    void Init(const char* argv0);
    void Done();

    class InputBuffer;
    class OutputBuffer;

    class Reader : public std::istream
    {
    public:
        Reader(const std::string& filename = "");
        ~Reader();

        bool is_open() const;
        bool open(const std::string& filename);
        void close();

    private:
        InputBuffer* _buf;
    };

    class Writer : public std::ostream
    {
    public:
        Writer(const std::string& filename = "", bool append = false);
        ~Writer();

        bool is_open() const;
        bool open(const std::string& filename, bool append = false);
        void close();

    private:
        OutputBuffer* _buf;
    };

};

#endif
