#ifndef __VIDEO_EXT_H__
#define __VIDEO_EXT_H__

#include <GL/glfw.h>
#include <GL/glext.h>

#ifndef GL_ARB_multitexture_
extern PFNGLACTIVETEXTUREARBPROC           glActiveTextureARB;
#endif

/*
PFNGLGENPROGRAMSARBPROC glGenProgramsARB;
PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;
PFNGLGETPROGRAMIVARBPROC glGetProgramivARB;
PFNGLDELETEPROGRAMPROC glDeleteProgramsARB;
PFNGLBINDPROGRAMARBPROC glBindProgramARB;
*/

#ifndef GL_ARB_shader_objects_
extern PFNGLCREATESHADEROBJECTARBPROC      glCreateShaderObjectARB;
extern PFNGLSHADERSOURCEARBPROC            glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC           glCompileShaderARB;

extern PFNGLCREATEPROGRAMOBJECTARBPROC     glCreateProgramObjectARB;
extern PFNGLATTACHOBJECTARBPROC            glAttachObjectARB;
extern PFNGLLINKPROGRAMARBPROC             glLinkProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC        glUseProgramObjectARB;

extern PFNGLGETOBJECTPARAMETERIVARBPROC    glGetObjectParameterivARB;
extern PFNGLGETINFOLOGARBPROC              glGetInfoLogARB;

extern PFNGLDETACHOBJECTARBPROC            glDetachObjectARB;
extern PFNGLDELETEOBJECTARBPROC            glDeleteObjectARB;

extern PFNGLGETUNIFORMLOCATIONARBPROC      glGetUniformLocationARB;
extern PFNGLUNIFORM1IARBPROC               glUniform1iARB;
extern PFNGLUNIFORM3FARBPROC               glUniform3fARB;
extern PFNGLUNIFORMMATRIX4FVARBPROC        glUniformMatrix4fvARB;
extern PFNGLVERTEXATTRIB2FARBPROC          glVertexAttrib2fARB;
extern PFNGLVERTEXATTRIB3FVARBPROC         glVertexAttrib3fvARB;
#endif

#ifndef GL_EXT_framebuffer_object_
extern PFNGLGENFRAMEBUFFERSEXTPROC         glGenFramebuffersEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC         glBindFramebufferEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    glFramebufferTexture2DEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  glCheckFramebufferStatusEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC      glDeleteFramebuffersEXT;
#endif

#ifndef GL_ARB_vertex_buffer_object_
extern PFNGLGENBUFFERSARBPROC              glGenBuffersARB;
extern PFNGLBINDBUFFERARBPROC              glBindBufferARB;
extern PFNGLBUFFERDATAARBPROC              glBufferDataARB;
extern PFNGLDELETEBUFFERSARBPROC           glDeleteBuffersARB;
extern PFNGLBUFFERSUBDATAARBPROC           glBufferSubDataARB;
#endif

template <typename T>
void loadProcAddress(const char* name, T& proc)
{
    proc = reinterpret_cast<T>(glfwGetProcAddress(name));
    if (proc == NULL)
    {
        throw Exception("Address of '" + string(name) + "' not found");
    }
}

#define loadProc(X) loadProcAddress(#X, X)

#endif
