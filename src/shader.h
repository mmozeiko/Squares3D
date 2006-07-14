#ifndef __SHADER_H__
#define __SHADER_H__

class Video;

class Shader
{
    friend class Video;

private:
    Shader(const string& vp, const string& fp);
    ~Shader();

    void begin() const;
    void end() const;

    void checkShaderStatus(unsigned int handle, int status);

    unsigned int m_vhandle;
    unsigned int m_fhandle;

public:
    unsigned int m_program;
};

#endif
