#include "video_ext.h"

#ifndef GL_ARB_multitexture
PFNGLACTIVETEXTUREARBPROC           glActiveTextureARB = NULL;
#endif

#ifndef GL_EXT_framebuffer_object
PFNGLGENFRAMEBUFFERSEXTPROC         glGenFramebuffersEXT = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC         glBindFramebufferEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    glFramebufferTexture2DEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  glCheckFramebufferStatusEXT = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC      glDeleteFramebuffersEXT = NULL;
#endif

#ifndef GL_ARB_vertex_buffer_object
PFNGLGENBUFFERSARBPROC              glGenBuffersARB = NULL;
PFNGLBINDBUFFERARBPROC              glBindBufferARB = NULL;
PFNGLBUFFERDATAARBPROC              glBufferDataARB = NULL;
PFNGLDELETEBUFFERSARBPROC           glDeleteBuffersARB = NULL;
PFNGLBUFFERSUBDATAARBPROC           glBufferSubDataARB = NULL;
#endif
