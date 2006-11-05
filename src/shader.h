#ifndef __SHADER_H__
#define __SHADER_H__

#include "common.h"
#include "vmath.h"
#include "video.h"

class Video;

class Shader : public NoCopy
{
public:
    Shader(const string& name);
    ~Shader();

    void begin() const;
    void end() const;

    void setInt1(const string& name, int value) const;
    void setFloat1(const string& name, float value) const;
    void setFloat4(const string& name, const Vector& value) const;

    void checkShaderStatus(GLhandleARB handle, int status);

    GLhandleARB m_vhandle;
    GLhandleARB m_fhandle;
    GLhandleARB m_program;
};

#endif
