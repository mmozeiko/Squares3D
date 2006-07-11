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
    InputBuffer() : std::streambuf(), m_handle(NULL)
    {
    }

    ~InputBuffer()
    {
        close();
    }

    bool is_open() const
    {
        return m_handle != NULL;
    }

    bool open(const std::string filename)
    {
        if (m_handle!=NULL)
        {
            close();
        }

        m_handle = PHYSFS_openRead(filename.c_str());

        if (m_handle!=NULL)
        {
            setg(_buffer, _buffer, _buffer);
        }
        return m_handle!=NULL;
    }

    void close()
    {
        if (m_handle != NULL)
        {
            PHYSFS_close(m_handle);
            setg(0, 0, 0);
            m_handle = NULL;
        }
    }

    unsigned int filesize() const
    {
        if (m_handle != NULL)
        {
            return static_cast<unsigned int>(PHYSFS_fileLength(m_handle));
        }
        return 0;
    }

protected:
    int underflow()
    {
        int bytes_read = static_cast<int>(PHYSFS_read(m_handle, _buffer, 1, BUFSIZE));
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
            if (PHYSFS_read(m_handle, s+cnt, 1, n) != n)
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
        int curbpos = static_cast<int>(PHYSFS_tell(m_handle));
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
            if (PHYSFS_seek(m_handle, sp)==0)
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
        int curbpos = static_cast<int>(PHYSFS_tell(m_handle));
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
            newpos = static_cast<int>(PHYSFS_fileLength(m_handle))-off;
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
            if (PHYSFS_seek(m_handle, newpos)==0)
            {
                return EOF;
            }
            setg(_buffer, _buffer, _buffer);
        }
        return newpos;
    }

private:
    PHYSFS_File* m_handle;
    char _buffer[BUFSIZE];
};

class OutputBuffer : public std::streambuf
{
public:
    OutputBuffer() : std::streambuf(), m_handle(NULL)
    {
    }

    ~OutputBuffer()
    {
        close();
    }

    bool is_open() const
    {
        return m_handle != NULL;
    }

    bool open(const std::string filename, bool append)
    {
        if (m_handle != NULL)
        {
            close();
        }

        if (append)
        {
            m_handle = PHYSFS_openAppend(filename.c_str());
        }
        else
        {
            m_handle = PHYSFS_openWrite(filename.c_str());
        }
        if (m_handle != NULL)
        {
            setp(_buffer, _buffer + BUFSIZE);
        }
        return m_handle != NULL;
    }

    void close()
    {
        if (m_handle != NULL)
        {
            sync();
            PHYSFS_close(m_handle);
            setp(0, 0);
            m_handle = NULL;
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
            if (PHYSFS_write(m_handle, pbase(), 1, static_cast<unsigned int>(pptr()-pbase())) != pptr()-pbase())
            {
                return EOF;
            }
            PHYSFS_flush(m_handle);
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
        if (PHYSFS_write(m_handle, s, 1, n) != n)
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
            return static_cast<int>(PHYSFS_tell(m_handle));
        }
        return EOF;
    }

private:
    PHYSFS_File* m_handle;
    char _buffer[BUFSIZE];
};

// PUBLIC part

Reader::Reader(const std::string& filename) : 
    m_buf(new InputBuffer()), 
    std::istream(std::_Uninitialized())
{
    clear();
    init(m_buf);
    if (!filename.empty())
    {
        m_buf->open(filename);
    }
}

Reader::~Reader()
{
    delete m_buf;
}

bool Reader::is_open() const
{
    return m_buf->is_open();
}

bool Reader::open(const std::string& filename)
{
    return m_buf->open(filename);
}

void Reader::close()
{
    m_buf->close();
}

unsigned int Reader::filesize() const
{
    return m_buf->filesize();
}

Writer::Writer(const std::string& filename, bool append) : 
    m_buf(new OutputBuffer()), 
    std::ostream(std::_Uninitialized())
{
    clear();
    init(m_buf);
    if (!filename.empty())
    {
        m_buf->open(filename, append);
    }
}

Writer::~Writer()
{
    delete m_buf;
}

bool Writer::is_open() const
{
    return m_buf->is_open();
}

bool Writer::open(const std::string& filename, bool append)
{
    return m_buf->open(filename, append);
}

void Writer::close()
{
    m_buf->close();
}

void init(const char* argv0)
{
    clog << "Initializing filesystem." << endl;

    if (PHYSFS_init(argv0)==0 ||
        PHYSFS_setWriteDir(PHYSFS_getBaseDir())==0)
    {
        throw Exception(PHYSFS_getLastError());
    }

    if (PHYSFS_mount("data.zip", "/data", 1) == 0 ||
        PHYSFS_mount(PHYSFS_getBaseDir(), "/", 1) == 0)
    {
        throw Exception(PHYSFS_getLastError());
    }
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
