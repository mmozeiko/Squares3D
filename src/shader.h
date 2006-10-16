#ifndef __SHADER_H__
#define __SHADER_H__

#include <GL/glfw.h>
#include <GL/glext.h>
#include "common.h"

class Video;

class Shader : NoCopy
{
    friend class Video;

private:
    Shader(const string& vp, const string& fp);
    ~Shader();

    void begin() const;
    void end() const;


    void checkShaderStatus(GLhandleARB handle, int status);

    GLhandleARB m_vhandle;
    GLhandleARB m_fhandle;

public:
    GLhandleARB m_program;
};

#endif
