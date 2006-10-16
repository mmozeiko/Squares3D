#ifndef __VIDEO_EXT_H__
#define __VIDEO_EXT_H__

#include <GL/glfw.h>
#include <GL/glext.h>

#ifdef WIN32
#define PPAPIENTRYP APIENTRY *
#endif
#ifndef PPAPIENTRYP
#define PPAPIENTRYP *
#endif

typedef void (PPAPIENTRYP FNGLACTIVETEXTUREARBPROC) (GLenum texture);

typedef GLhandleARB (PPAPIENTRYP FNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
typedef void (PPAPIENTRYP FNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
typedef void (PPAPIENTRYP FNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);

typedef GLhandleARB (PPAPIENTRYP FNGLCREATEPROGRAMOBJECTARBPROC) (void);
typedef void (PPAPIENTRYP FNGLATTACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB obj);
typedef void (PPAPIENTRYP FNGLLINKPROGRAMARBPROC) (GLhandleARB programObj);
typedef void (PPAPIENTRYP FNGLUSEPROGRAMOBJECTARBPROC) (GLhandleARB programObj);
typedef void (PPAPIENTRYP FNGLGETOBJECTPARAMETERIVARBPROC) (GLhandleARB obj, GLenum pname, GLint *params);
typedef void (PPAPIENTRYP FNGLGETINFOLOGARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
typedef void (PPAPIENTRYP FNGLDETACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB attachedObj);
typedef void (PPAPIENTRYP FNGLDELETEOBJECTARBPROC) (GLhandleARB obj);

typedef GLint (PPAPIENTRYP FNGLGETUNIFORMLOCATIONARBPROC) (GLhandleARB programObj, const GLcharARB *name);
typedef void (PPAPIENTRYP FNGLUNIFORM1IARBPROC) (GLint location, GLint v0);
typedef void (PPAPIENTRYP FNGLUNIFORM3FARBPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (PPAPIENTRYP FNGLUNIFORMMATRIX4FVARBPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (PPAPIENTRYP FNGLVERTEXATTRIB2FARBPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (PPAPIENTRYP FNGLVERTEXATTRIB3FVARBPROC) (GLuint index, const GLfloat *v);

typedef void (PPAPIENTRYP FNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef void (PPAPIENTRYP FNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
typedef void (PPAPIENTRYP FNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef GLenum (PPAPIENTRYP FNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
typedef void (PPAPIENTRYP FNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);

typedef void (PPAPIENTRYP FNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (PPAPIENTRYP FNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (PPAPIENTRYP FNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
typedef void (PPAPIENTRYP FNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (PPAPIENTRYP FNGLBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);

#endif
