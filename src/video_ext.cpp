#include "video_ext.h"

//#ifndef GL_ARB_multitexture
PFNGLACTIVETEXTUREARBPROC           pglActiveTextureARB = NULL;
//#endif

//#ifndef GL_EXT_framebuffer_object
PFNGLGENFRAMEBUFFERSEXTPROC         pglGenFramebuffersEXT = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC         pglBindFramebufferEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    pglFramebufferTexture2DEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  pglCheckFramebufferStatusEXT = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC      pglDeleteFramebuffersEXT = NULL;
//#endif

//#ifndef GL_ARB_vertex_buffer_object
PFNGLGENBUFFERSARBPROC              pglGenBuffersARB = NULL;
PFNGLBINDBUFFERARBPROC              pglBindBufferARB = NULL;
PFNGLBUFFERDATAARBPROC              pglBufferDataARB = NULL;
PFNGLDELETEBUFFERSARBPROC           pglDeleteBuffersARB = NULL;
PFNGLBUFFERSUBDATAARBPROC           pglBufferSubDataARB = NULL;
//#endif
