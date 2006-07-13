#include "video.h"
#include "shader.h"

Shader::Shader(const string& vp, const string& fp)
{
    Video::glGenProgramsARB(1, &m_fhandle);
    Video::glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, 
            static_cast<GLsizei>(fp.size()), fp.c_str());
        
    GLint errorPos, isNative;
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
    Video::glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
    if ((errorPos != -1) || (isNative != 1))
    {
        // TODO: shader program is not native!
        string error = reinterpret_cast<const char*>(glGetString(GL_PROGRAM_ERROR_STRING_ARB));
        throw Exception(error);
    }



    Video::glGenProgramsARB(1, &m_vhandle);
    Video::glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, 
            static_cast<GLsizei>(vp.size()), vp.c_str());
        
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
    Video::glGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
    if ((errorPos != -1) || (isNative != 1))
    {
        // TODO: shader program is not native!
        string error = reinterpret_cast<const char*>(glGetString(GL_PROGRAM_ERROR_STRING_ARB));
        throw Exception(error);
    }
    
}

Shader::~Shader()
{
    Video::glDeleteProgramsARB(m_vhandle);
    Video::glDeleteProgramsARB(m_fhandle);
}

void Shader::begin() const
{
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    glEnable(GL_VERTEX_PROGRAM_ARB);
    Video::glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_fhandle);
    Video::glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_vhandle);
}

void Shader::end() const
{
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    glDisable(GL_VERTEX_PROGRAM_ARB);
}

