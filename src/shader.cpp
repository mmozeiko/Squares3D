#include "shader.h"
#include "video.h"
#include "video_ext.h"

/*Shader::Shader(const string& vp, const string& fp)
{
    Video::glGenProgramsARB(1, &m_fhandle);
    Video::glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_fhandle);
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
    Video::glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_vhandle);
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
*/

Shader::Shader(const string& vp, const string& fp)
{
    const char* shaders[2] = { vp.c_str(), fp.c_str() };
    GLhandleARB objs[2] = {
        m_vhandle = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB),
        m_fhandle = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB),
    };

    for (int i=0; i<2; ++i)
    {
        glShaderSourceARB(objs[i], 1, static_cast<const GLcharARB**>(&shaders[i]), NULL);
        glCompileShaderARB(objs[i]);
        checkShaderStatus(objs[i], GL_OBJECT_COMPILE_STATUS_ARB);
    }

    m_program = glCreateProgramObjectARB();
    glAttachObjectARB(m_program, objs[0]);
    glAttachObjectARB(m_program, objs[1]);

    glLinkProgramARB(m_program);

    glDeleteObjectARB(objs[0]);
    glDeleteObjectARB(objs[1]);

    checkShaderStatus(m_program, GL_OBJECT_LINK_STATUS_ARB);
    glUseProgramObjectARB(m_program);

    GLint decalMap = glGetUniformLocationARB(m_program, "decalMap"); 
    GLint heightMap = glGetUniformLocationARB(m_program, "heightMap"); 

    glUniform1iARB(decalMap, 0);
    glUniform1iARB(heightMap, 1);
    glUseProgramObjectARB(0);
}

void Shader::checkShaderStatus(GLhandleARB handle, int status)
{
    int param;
    glGetObjectParameterivARB(handle, status, (GLint*)&param);
    if (param == 1)
    {
        return;
    }

    int infologLength;
    glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, (GLint*)&infologLength);

    if (infologLength > 0)
    {
        vector<char> infoLog(infologLength);
        int charsWritten;
        glGetInfoLogARB(handle, infologLength, (GLsizei*)&charsWritten, &infoLog[0]);
        string error(infoLog.begin(), infoLog.end());
        throw Exception(error);
    }
    else
    {
        throw Exception("Unknown error");
    }
}

Shader::~Shader()
{
    glDeleteObjectARB(m_program);
}

void Shader::begin() const
{
    glUseProgramObjectARB(m_program);
}

void Shader::end() const
{
    glUseProgramObjectARB(0);
}
