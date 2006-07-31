#include "shader.h"
#include "video.h"

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
        m_vhandle = Video::glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB),
        m_fhandle = Video::glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB),
    };

    for (int i=0; i<2; ++i)
    {
        Video::glShaderSourceARB(objs[i], 1, static_cast<const GLcharARB**>(&shaders[i]), NULL);
        Video::glCompileShaderARB(objs[i]);
        checkShaderStatus(objs[i], GL_OBJECT_COMPILE_STATUS_ARB);
    }

    m_program = Video::glCreateProgramObjectARB();
    Video::glAttachObjectARB(m_program, objs[0]);
    Video::glAttachObjectARB(m_program, objs[1]);

    Video::glLinkProgramARB(m_program);

    Video::glDeleteObjectARB(objs[0]);
    Video::glDeleteObjectARB(objs[1]);

    checkShaderStatus(m_program, GL_OBJECT_LINK_STATUS_ARB);
    Video::glUseProgramObjectARB(m_program);

    GLint decalMap = Video::glGetUniformLocationARB(m_program, "decalMap"); 
    GLint heightMap = Video::glGetUniformLocationARB(m_program, "heightMap"); 

    Video::glUniform1iARB(decalMap, 0);
    Video::glUniform1iARB(heightMap, 1);
    Video::glUseProgramObjectARB(0);
}

void Shader::checkShaderStatus(unsigned int handle, int status)
{
    int param;
    Video::glGetObjectParameterivARB(handle, status, &param);
    if (param == 1)
    {
        return;
    }

    int infologLength;
    Video::glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength);

    if (infologLength > 0)
    {
        vector<char> infoLog(infologLength);
        int charsWritten;
        Video::glGetInfoLogARB(handle, infologLength, &charsWritten, &infoLog[0]);
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
    Video::glDeleteObjectARB(m_program);
}

void Shader::begin() const
{
    Video::glUseProgramObjectARB(m_program);
}

void Shader::end() const
{
    Video::glUseProgramObjectARB(0);
}
