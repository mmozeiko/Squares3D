#ifndef __FILE_H__
#define __FILE_H__

#include <cstdio>

#include "common.h"

namespace File
{
    void init(const char* argv0);
    void done();
    bool exists(const string& filename);

    class File : NoCopy
    {
    public:
        virtual ~File();
        void seek(int position, int type = SEEK_SET);
        bool is_open();
        size_t tell();
        size_t size();
        void close();

    protected:
        struct _PHYSFS_File;
        _PHYSFS_File* m_handle;

        File(_PHYSFS_File* handle);
    };

    class Reader : public File
    {
    public:
        Reader(const string& filename);
        void open(const string& filename);

        size_t read(void* buffer, size_t size);
        bool eof();
    };

    class Writer : public File
    {
    public:
        Writer(const string& filename, bool append = false);

        size_t write(const void* buffer, size_t size);
    };

};

#endif
