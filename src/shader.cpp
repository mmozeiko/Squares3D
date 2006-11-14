#include "shader.h"
#include "video.h"
#include "file.h"

Shader::Shader(const string& name) : m_program(0)
{
    string shaders[2];

    string vp_filename = "/data/shaders/" + name + ".vsh.glsl";
    string fp_filename = "/data/shaders/" + name + ".fsh.glsl";

    File::Reader file(vp_filename);
    if (!file.is_open())
    {
        clog << Exception("Shader '" + vp_filename + "' not found") << endl;
        return ;
    }
    vector<char> v(file.size());
    file.read(&v[0], file.size());
    file.close();
    shaders[0].assign(v.begin(), v.end());

    file.open(fp_filename);
    if (!file.is_open())
    {
        clog << Exception("Shader '" + fp_filename + "' not found") << endl;
        return ;
    }
    v.resize(file.size());
    file.read(&v[0], file.size());
    file.close();
    shaders[1].assign(v.begin(), v.end());
        
    GLhandleARB objs[2] = {
        glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB),
        glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB),
    };

    for (int i=0; i<2; ++i)
    {
        const char* cstr = shaders[i].c_str();
        glShaderSourceARB(objs[i], 1, static_cast<const GLcharARB**>(&cstr), NULL);
        glCompileShaderARB(objs[i]);
        if (!checkShaderStatus(objs[i], GL_OBJECT_COMPILE_STATUS_ARB))
        {
            glDeleteObjectARB(objs[0]);
            glDeleteObjectARB(objs[1]);
            return;
        }
    }

    m_program = glCreateProgramObjectARB();
    glAttachObjectARB(m_program, objs[0]);
    glAttachObjectARB(m_program, objs[1]);

    glLinkProgramARB(m_program);

    glDeleteObjectARB(objs[0]);
    glDeleteObjectARB(objs[1]);

    if (!checkShaderStatus(m_program, GL_OBJECT_LINK_STATUS_ARB))
    {
        glDeleteObjectARB(m_program);
        m_program = 0;
        return;
    }
    glUseProgramObjectARB(m_program);
}

bool Shader::checkShaderStatus(GLhandleARB handle, int status)
{
    int param;
    glGetObjectParameterivARB(handle, status, (GLint*)&param);
    if (param == 1)
    {
        return true;
    }

    int infologLength;
    glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, (GLint*)&infologLength);

    if (infologLength > 0)
    {
        vector<char> infoLog(infologLength);
        int charsWritten;
        glGetInfoLogARB(handle, infologLength, (GLsizei*)&charsWritten, &infoLog[0]);
        string error(infoLog.begin(), infoLog.end());
        clog << Exception(error) << endl;
        return false;
    }
    else
    {
        clog << Exception("Unknown error") << endl;
        return false;
    }
}

Shader::~Shader()
{
    if (m_program != 0)
    {
        glDeleteObjectARB(m_program);
    }
}

bool Shader::valid() const
{
    return m_program != 0;
}

void Shader::begin() const
{
    if (m_program != 0)
    {
        glUseProgramObjectARB(m_program);
    }
}

void Shader::end() const
{
    if (m_program != 0)
    {
        glUseProgramObjectARB(0);
    }
}

void Shader::setInt1(const string& name, int value) const
{
    if (m_program != 0)
    {
        GLint loc = glGetUniformLocationARB(m_program, name.c_str()); 
        glUniform1iARB(loc, value);
    }
}

void Shader::setFloat1(const string& name, float value) const
{
    if (m_program != 0)
    {
        GLint loc = glGetUniformLocationARB(m_program, name.c_str()); 
        glUniform1fARB(loc, value);
    }
}

void Shader::setFloat4(const string& name, const Vector& value) const
{
    if (m_program != 0)
    {
        GLint loc = glGetUniformLocationARB(m_program, name.c_str()); 
        glUniform4fvARB(loc, 1, value.v);
    }
}
