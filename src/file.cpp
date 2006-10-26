#include <stddef.h>
#include <physfs.h>

#pragma warning(disable : 4996)

#include <algorithm>
#include "file.h"

// PRIVATE part

static const size_t BUFSIZE = 4096;

namespace File
{

    struct File::_PHYSFS_File : public PHYSFS_File {};

    File::File(_PHYSFS_File* handle) : m_handle(handle)
    {
        if (m_handle != NULL && PHYSFS_setBuffer(m_handle, BUFSIZE) == 0)
        {
            throw Exception(PHYSFS_getLastError());
        }
    }

    void File::seek(int position, int type)
    {
        size_t pos;
        if (type == SEEK_CUR)
        {
            pos = tell() - position;
        }
        else if (type == SEEK_END)
        {
            pos = size() + position;
        }
        else // SEEK_BEG
        {
            pos = static_cast<size_t>(position);
        }

        if (PHYSFS_seek(m_handle, pos) == 0)
        {
            throw Exception(PHYSFS_getLastError());
        }
    }
    
    bool File::is_open()
    {
        return m_handle != NULL;
    }

    size_t File::tell()
    {
        long long curpos = PHYSFS_tell(m_handle);

        if (curpos == -1)
        {
            throw Exception(PHYSFS_getLastError());
        }

        return static_cast<size_t>(curpos);
    }

    size_t File::size()
    {
        long long filesize = PHYSFS_fileLength(m_handle);

        if (filesize == -1)
        {
            throw Exception(PHYSFS_getLastError());
        }

        return static_cast<size_t>(filesize);
    }

    void File::close()
    {
        if (m_handle == NULL)
        {
            return;
        }

        if (PHYSFS_flush(m_handle)==0)
        {
            throw Exception(PHYSFS_getLastError());
        }

        if (PHYSFS_close(m_handle)==0)
        {
            throw Exception(PHYSFS_getLastError());
        }

        m_handle = NULL;
    }

    File::~File()
    {
        if (m_handle != NULL)
        {
            close();
        }
    }

    Reader::Reader(const string& filename) :
        File(static_cast<_PHYSFS_File*>(PHYSFS_openRead(filename.c_str())))
    {
    }

    void Reader::open(const string& filename)
    {
        close();
        m_handle = static_cast<_PHYSFS_File*>(PHYSFS_openRead(filename.c_str()));
    }

    size_t Reader::read(void* buffer, size_t size)
    {
        long long result = PHYSFS_read(m_handle, buffer, 1, static_cast<PHYSFS_uint32>(size));

        if (result == -1)
        {
            throw Exception(PHYSFS_getLastError());
        }

        return static_cast<size_t>(result);
    }

    bool Reader::eof()
    {
        return PHYSFS_eof(m_handle) != 0;
    }

    Writer::Writer(const string& filename, bool append) :
        File(static_cast<_PHYSFS_File*>(
            append ? PHYSFS_openAppend(filename.c_str()) : PHYSFS_openWrite(filename.c_str())
        ))
    {
    }

    size_t Writer::write(const void* buffer, size_t size)
    {
        long long result = PHYSFS_write(m_handle, buffer, 1, static_cast<PHYSFS_uint32>(size));

        if (result == -1)
        {
            throw Exception(PHYSFS_getLastError());
        }

        return static_cast<size_t>(result);
    }

    string getBase(const char* argv0, bool dirSep)
    {
        const string tmp(argv0);
        const string ds(PHYSFS_getDirSeparator());
        size_t idx = tmp.size()-1-ds.size();

        string base;

        while (idx > 0)
        {
            bool found = true;
            for (size_t i = 0; i < ds.size(); i++)
            {
                if (tmp[idx+i] != ds[i])
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                base = tmp.substr(0, idx);
                break;
            }
            idx--;
        }
        return base + (dirSep ? ds : "");
    }

    void init(const char* argv0)
    {
        clog << "Initializing filesystem." << endl;
        clog << "Base directory: " << getBase(argv0) << endl;

        if (PHYSFS_init(argv0)==0)
        {
            throw Exception(PHYSFS_getLastError());
        }

        if (PHYSFS_setWriteDir(getBase(argv0).c_str())==0)
        {
            throw Exception(PHYSFS_getLastError());
        }

        if (PHYSFS_mount((getBase(argv0, true) + "data.zip").c_str(), "/data", 1) == 0)
        {
            throw Exception(PHYSFS_getLastError());
        }

        if (PHYSFS_mount(getBase(argv0).c_str(), "/", 0) == 0)
        {
            throw Exception(PHYSFS_getLastError());
        }
    }

    void done()
    {
        clog << "Closing filesystem." << endl;

        if (PHYSFS_deinit()==0)
        {
            clog << "ERROR: " << Exception(PHYSFS_getLastError()) << endl;
        }
    }

    bool exists(const string& filename)
    {
        return PHYSFS_exists(filename.c_str()) != 0;
    }

}
