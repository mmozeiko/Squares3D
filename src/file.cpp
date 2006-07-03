#include <stddef.h>
#include <physfs.h>

#pragma warning(disable : 4996)

#include <streambuf>
#include <algorithm>

#include "file.h"
#include "common.h"

// PRIVATE part

static const size_t BUFSIZE = 4096;
namespace File
{

class InputBuffer : public std::streambuf
{
public:
    InputBuffer() : std::streambuf(), _handle(NULL)
    {
    }

    ~InputBuffer()
    {
        close();
    }

    bool is_open() const
    {
        return _handle != NULL;
    }

    bool open(const std::string filename)
    {
        if (_handle!=NULL)
        {
            close();
        }

        _handle = PHYSFS_openRead(filename.c_str());
        if (_handle!=NULL)
        {
            setg(_buffer, _buffer, _buffer);
        }
        return _handle!=NULL;
    }

    void close()
    {
        if (_handle != NULL)
        {
            PHYSFS_close(_handle);
            setg(0, 0, 0);
            _handle = NULL;
        }
    }

protected:
    int underflow()
    {
        int bytes_read = static_cast<int>(PHYSFS_read(_handle, _buffer, 1, BUFSIZE));
        if (bytes_read <= 0)
        {
            setg(_buffer, _buffer, _buffer);
            return EOF;
        }
        setg(_buffer, _buffer, _buffer + bytes_read);
        return traits_type::to_int_type(*(gptr()));
    }

    int showmanyc()
    {
        if (gptr() && ( gptr()<egptr() ))
        {
            return std::streamsize(egptr() - gptr());
        }
        return 0;
    }

    std::streamsize xsgetn(char *s, std::streamsize n)
    {
        int cnt = 0;
        if (gptr()<egptr())
        {
            cnt = std::min(n, static_cast<int>(egptr()-gptr()));
            std::copy(gptr(), gptr()+n, s);
            n -= cnt;
            setg(_buffer, _buffer+cnt+1, egptr());
        }
        else
        {
            setg(_buffer, _buffer, _buffer);
        }
        if (n > 0)
        {
            if (PHYSFS_read(_handle, s+cnt, 1, n) != n)
            {
                return EOF;
            }
            cnt += n;
        }
        return cnt;
    }

    std::streampos seekpos(
        std::streampos sp, 
        std::ios_base::openmode which = std::ios_base::in)
    {
        int curbpos = static_cast<int>(PHYSFS_tell(_handle));
        if (curbpos==-1)
        {
            return -1;
        }
        curbpos += -static_cast<int>(egptr()-eback());
        if ( sp>curbpos && sp<curbpos+static_cast<int>(egptr()-eback()) )
        {
            setg(_buffer, _buffer + (static_cast<int>(sp)-curbpos), egptr());
        }
        else
        {
            if (PHYSFS_seek(_handle, sp)==0)
            {
                return EOF;
            }
            setg(_buffer, _buffer, _buffer);
        }
        return sp;
    }

    std::streampos seekoff(
        std::streamoff off, 
        std::ios_base::seekdir way, 
        std::ios_base::openmode which = std::ios_base::in)
    {
        std::streampos newpos;
        int curbpos = static_cast<int>(PHYSFS_tell(_handle));
        if (curbpos==-1)
        {
            return EOF;
        }
        curbpos += -static_cast<int>(egptr()-eback());

        if (way==std::ios_base::beg)
        {
            newpos = off;
        }
        else if (way==std::ios_base::cur)
        {
            newpos = curbpos + off + (gptr()-eback());
        }
        else if (way==std::ios_base::end)
        {
            newpos = static_cast<int>(PHYSFS_fileLength(_handle))-off;
        }
        else
        {
            return EOF;
        }

        if ( newpos>=curbpos && newpos<curbpos+(egptr()-eback()) )
        {
            setg(_buffer, _buffer + (static_cast<int>(newpos)-curbpos), egptr());
        }
        else
        {
            if (PHYSFS_seek(_handle, newpos)==0)
            {
                return EOF;
            }
            setg(_buffer, _buffer, _buffer);
        }
        return newpos;
    }

private:
    PHYSFS_File* _handle;
    char _buffer[BUFSIZE];
};

class OutputBuffer : public std::streambuf
{
public:
    OutputBuffer() : std::streambuf(), _handle(NULL)
    {
    }

    ~OutputBuffer()
    {
        close();
    }

    bool is_open() const
    {
        return _handle != NULL;
    }

    bool open(const std::string filename, bool append)
    {
        if (_handle != NULL)
        {
            close();
        }

        if (append)
        {
            _handle = PHYSFS_openAppend(filename.c_str());
        }
        else
        {
            _handle = PHYSFS_openWrite(filename.c_str());
        }
        if (_handle != NULL)
        {
            setp(_buffer, _buffer + BUFSIZE);
        }
        return _handle != NULL;
    }

    void close()
    {
        if (_handle != NULL)
        {
            sync();
            PHYSFS_close(_handle);
            setp(0, 0);
            _handle = NULL;
        }
    }

protected:
    int overflow(int c)
    {
        if (!pptr() || c==EOF)
        {
            return EOF;
        }

        if (epptr()-pptr()<=0)
        {
            if (sync()==EOF)
            {
                return EOF;
            }
            setp(_buffer, _buffer + BUFSIZE);
        }
        *pptr() = static_cast<char>(c);
        pbump(1);
        return traits_type::not_eof(c);
    }

    int sync()
    {
        if (pptr() > pbase())
        {
            if (PHYSFS_write(_handle, pbase(), 1, static_cast<unsigned int>(pptr()-pbase())) != pptr()-pbase())
            {
                return EOF;
            }
            PHYSFS_flush(_handle);
            setp(_buffer, _buffer + BUFSIZE);
        }
        return 0;
    }
  
    std::streamsize xsputn(char *s, std::streamsize n)
    {
        if (sync()==EOF)
        {
            return EOF;
        }
        if (PHYSFS_write(_handle, s, 1, n) != n)
        {
            return EOF;
        }
        setp(_buffer, _buffer + BUFSIZE);
        return n;
    }

    std::streampos seekoff(
        std::streamoff off, 
        std::ios_base::seekdir way, 
        std::ios_base::openmode which = std::ios_base::out)
    {
        if (off==0 && way==std::ios_base::cur)
        {
            return static_cast<int>(PHYSFS_tell(_handle));
        }
        return EOF;
    }

private:
    PHYSFS_File* _handle;
    char _buffer[BUFSIZE];
};

// PUBLIC part

Reader::Reader(const std::string& filename) : 
    _buf(new InputBuffer()), 
    std::istream(std::_Uninitialized())
{
    clear();
    init(_buf);
    if (!filename.empty())
    {
        _buf->open(filename);
    }
}

Reader::~Reader()
{
    delete _buf;
}

bool Reader::is_open() const
{
    return _buf->is_open();
}

bool Reader::open(const std::string& filename)
{
    return _buf->open(filename);
}

void Reader::close()
{
    _buf->close();
}

Writer::Writer(const std::string& filename, bool append) : 
    _buf(new OutputBuffer()), 
    std::ostream(std::_Uninitialized())
{
    clear();
    init(_buf);
    if (!filename.empty())
    {
        _buf->open(filename, append);
    }
}

Writer::~Writer()
{
    delete _buf;
}

bool Writer::is_open() const
{
    return _buf->is_open();
}

bool Writer::open(const std::string& filename, bool append)
{
    return _buf->open(filename, append);
}

void Writer::close()
{
    _buf->close();
}

void init(const char* argv0)
{
    clog << "Initializing filesystem." << endl;

    if (PHYSFS_init(argv0)==0 ||
        PHYSFS_setWriteDir(PHYSFS_getBaseDir())==0 ||
        PHYSFS_mount(PHYSFS_getBaseDir(), NULL, 1)==0)
    {
        throw Exception(PHYSFS_getLastError());
    }

    PHYSFS_mount("data.zip", "data", 1);
}

void done()
{
    clog << "Closing filesystem." << endl;

    if (PHYSFS_deinit()==0)
    {
        throw Exception(PHYSFS_getLastError());
    }
}

}
