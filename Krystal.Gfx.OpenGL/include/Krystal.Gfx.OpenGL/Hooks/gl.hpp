#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Detection.hpp"

#ifndef KRYS_GL_OVERRIDE_FUNCTIONS
  #define KRYS_GL_OVERRIDE_FUNCTIONS 1
#endif

#pragma region OpenGL Extensions Header

/*
  GL extensions are defined in glcorearb.h, which is included below. It uses a series of defines to add new
  function pointer types and symbols for OpenGL functions, only including them if they are not already
  defined. We can remove extensions that we do not support or need in our application by defining them before
  including glcorearb.h. This doesn't have any real effect on the code, but it allows us to
  stop the irrelevant symbols from polluting our namespace.
 */

#ifndef KRYS_GL_INCLUDE_ALL_EXTENSIONS
  #define GL_ARB_ES2_compatibility 1
  #define GL_ARB_ES3_compatibility 1
  #define GL_ARB_ES3_1_compatibility 1
  #define GL_ARB_ES3_2_compatibility 1
#endif

#include <Krystal.Gfx.OpenGL/Official/glcorearb.h>

#pragma endregion

namespace Krys::Gfx::OpenGL
{
  /// @brief Initializes OpenGL hooks.
  KRYS_NODISCARD bool InitialiseGLHooks() noexcept;

  /// @brief Checks if a specific OpenGL extension is supported.
  KRYS_NODISCARD bool IsGLExtensionSupported(const char *extension) noexcept;
}

// OpenGL hook declarations
namespace Krys::Gfx::OpenGL
{
#pragma region GL Core Functions

#pragma region 1.0

  void glCullFace(GLenum mode) noexcept;

  void glFrontFace(GLenum mode) noexcept;

  void glHint(GLenum target, GLenum mode) noexcept;

  void glLineWidth(GLfloat width) noexcept;

  void glPointSize(GLfloat size) noexcept;

  void glPolygonMode(GLenum face, GLenum mode) noexcept;

  void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) noexcept;

  void glTexParameterf(GLenum target, GLenum pname, GLfloat param) noexcept;

  void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params) noexcept;

  void glTexParameteri(GLenum target, GLenum pname, GLint param) noexcept;

  void glTexParameteriv(GLenum target, GLenum pname, const GLint *params) noexcept;

  void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border,
                    GLenum format, GLenum type, const void *pixels) noexcept;

  void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
                    GLint border, GLenum format, GLenum type, const void *pixels) noexcept;

  void glDrawBuffer(GLenum buf) noexcept;

  void glClear(GLbitfield mask) noexcept;

  void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) noexcept;

  void glClearStencil(GLint s) noexcept;

  void glClearDepth(GLdouble depth) noexcept;

  void glStencilMask(GLuint mask) noexcept;

  void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) noexcept;

  void glDepthMask(GLboolean flag) noexcept;

  void glDisable(GLenum cap) noexcept;

  void glEnable(GLenum cap) noexcept;

  void glFinish() noexcept;

  void glFlush() noexcept;

  void glBlendFunc(GLenum sfactor, GLenum dfactor) noexcept;

  void glLogicOp(GLenum opcode) noexcept;

  void glStencilFunc(GLenum func, GLint ref, GLuint mask) noexcept;

  void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) noexcept;

  void glDepthFunc(GLenum func) noexcept;

  void glPixelStoref(GLenum pname, GLfloat param) noexcept;

  void glPixelStorei(GLenum pname, GLint param) noexcept;

  void glReadBuffer(GLenum src) noexcept;

  void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
                    void *pixels) noexcept;

  void glGetBooleanv(GLenum pname, GLboolean *data) noexcept;

  void glGetDoublev(GLenum pname, GLdouble *data) noexcept;

  KRYS_NODISCARD GLenum glGetError() noexcept;

  void glGetFloatv(GLenum pname, GLfloat *data) noexcept;

  void glGetIntegerv(GLenum pname, GLint *data) noexcept;

  KRYS_NODISCARD const GLubyte *glGetString(GLenum name) noexcept;

  void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void *pixels) noexcept;

  void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params) noexcept;

  void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params) noexcept;

  void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params) noexcept;

  void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params) noexcept;

  KRYS_NODISCARD GLboolean glIsEnabled(GLenum cap) noexcept;

  void glDepthRange(GLdouble n, GLdouble f) noexcept;

  void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glCullFace ::Krys::Gfx::OpenGL::glCullFace
  #define glFrontFace ::Krys::Gfx::OpenGL::glFrontFace
  #define glHint ::Krys::Gfx::OpenGL::glHint
  #define glLineWidth ::Krys::Gfx::OpenGL::glLineWidth
  #define glPointSize ::Krys::Gfx::OpenGL::glPointSize
  #define glPolygonMode ::Krys::Gfx::OpenGL::glPolygonMode
  #define glScissor ::Krys::Gfx::OpenGL::glScissor
  #define glTexParameterf ::Krys::Gfx::OpenGL::glTexParameterf
  #define glTexParameterfv ::Krys::Gfx::OpenGL::glTexParameterfv
  #define glTexParameteri ::Krys::Gfx::OpenGL::glTexParameteri
  #define glTexParameteriv ::Krys::Gfx::OpenGL::glTexParameteriv
  #define glTexImage1D ::Krys::Gfx::OpenGL::glTexImage1D
  #define glTexImage2D ::Krys::Gfx::OpenGL::glTexImage2D
  #define glDrawBuffer ::Krys::Gfx::OpenGL::glDrawBuffer
  #define glClear ::Krys::Gfx::OpenGL::glClear
  #define glClearColor ::Krys::Gfx::OpenGL::glClearColor
  #define glClearStencil ::Krys::Gfx::OpenGL::glClearStencil
  #define glClearDepth ::Krys::Gfx::OpenGL::glClearDepth
  #define glStencilMask ::Krys::Gfx::OpenGL::glStencilMask
  #define glColorMask ::Krys::Gfx::OpenGL::glColorMask
  #define glDepthMask ::Krys::Gfx::OpenGL::glDepthMask
  #define glDisable ::Krys::Gfx::OpenGL::glDisable
  #define glEnable ::Krys::Gfx::OpenGL::glEnable
  #define glFinish ::Krys::Gfx::OpenGL::glFinish
  #define glFlush ::Krys::Gfx::OpenGL::glFlush
  #define glBlendFunc ::Krys::Gfx::OpenGL::glBlendFunc
  #define glLogicOp ::Krys::Gfx::OpenGL::glLogicOp
  #define glStencilFunc ::Krys::Gfx::OpenGL::glStencilFunc
  #define glStencilOp ::Krys::Gfx::OpenGL::glStencilOp
  #define glDepthFunc ::Krys::Gfx::OpenGL::glDepthFunc
  #define glPixelStoref ::Krys::Gfx::OpenGL::glPixelStoref
  #define glPixelStorei ::Krys::Gfx::OpenGL::glPixelStorei
  #define glReadBuffer ::Krys::Gfx::OpenGL::glReadBuffer
  #define glReadPixels ::Krys::Gfx::OpenGL::glReadPixels
  #define glGetBooleanv ::Krys::Gfx::OpenGL::glGetBooleanv
  #define glGetDoublev ::Krys::Gfx::OpenGL::glGetDoublev
  #define glGetError ::Krys::Gfx::OpenGL::glGetError
  #define glGetFloatv ::Krys::Gfx::OpenGL::glGetFloatv
  #define glGetIntegerv ::Krys::Gfx::OpenGL::glGetIntegerv
  #define glGetString ::Krys::Gfx::OpenGL::glGetString
  #define glGetTexImage ::Krys::Gfx::OpenGL::glGetTexImage
  #define glGetTexParameterfv ::Krys::Gfx::OpenGL::glGetTexParameterfv
  #define glGetTexParameteriv ::Krys::Gfx::OpenGL::glGetTexParameteriv
  #define glGetTexLevelParameterfv ::Krys::Gfx::OpenGL::glGetTexLevelParameterfv
  #define glGetTexLevelParameteriv ::Krys::Gfx::OpenGL::glGetTexLevelParameteriv
  #define glIsEnabled ::Krys::Gfx::OpenGL::glIsEnabled
  #define glDepthRange ::Krys::Gfx::OpenGL::glDepthRange
  #define glViewport ::Krys::Gfx::OpenGL::glViewport
#endif

#pragma endregion

#pragma region 1.1

  void glDrawArrays(GLenum mode, GLint first, GLsizei count) noexcept;

  void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) noexcept;

  void glGetPointerv(GLenum pname, void **params) noexcept;

  void glPolygonOffset(GLfloat factor, GLfloat units) noexcept;

  void glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width,
                        GLint border) noexcept;

  void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width,
                        GLsizei height, GLint border) noexcept;

  void glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y,
                           GLsizei width) noexcept;

  void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
                           GLsizei width, GLsizei height) noexcept;

  void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type,
                       const void *pixels) noexcept;

  void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                       GLsizei height, GLenum format, GLenum type, const void *pixels) noexcept;

  void glBindTexture(GLenum target, GLuint texture) noexcept;

  void glDeleteTextures(GLsizei n, const GLuint *textures) noexcept;

  void glGenTextures(GLsizei n, GLuint *textures) noexcept;

  KRYS_NODISCARD GLboolean glIsTexture(GLuint texture) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glDrawArrays ::Krys::Gfx::OpenGL::glDrawArrays
  #define glDrawElements ::Krys::Gfx::OpenGL::glDrawElements
  #define glGetPointerv ::Krys::Gfx::OpenGL::glGetPointerv
  #define glPolygonOffset ::Krys::Gfx::OpenGL::glPolygonOffset
  #define glCopyTexImage1D ::Krys::Gfx::OpenGL::glCopyTexImage1D
  #define glCopyTexImage2D ::Krys::Gfx::OpenGL::glCopyTexImage2D
  #define glCopyTexSubImage1D ::Krys::Gfx::OpenGL::glCopyTexSubImage1D
  #define glCopyTexSubImage2D ::Krys::Gfx::OpenGL::glCopyTexSubImage2D
  #define glTexSubImage1D ::Krys::Gfx::OpenGL::glTexSubImage1D
  #define glTexSubImage2D ::Krys::Gfx::OpenGL::glTexSubImage2D
  #define glBindTexture ::Krys::Gfx::OpenGL::glBindTexture
  #define glDeleteTextures ::Krys::Gfx::OpenGL::glDeleteTextures
  #define glGenTextures ::Krys::Gfx::OpenGL::glGenTextures
  #define glIsTexture ::Krys::Gfx::OpenGL::glIsTexture
#endif

#pragma endregion

#pragma region 1.2

  void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type,
                           const void *indices) noexcept;

  void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
                    GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels) noexcept;

  void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width,
                       GLsizei height, GLsizei depth, GLenum format, GLenum type,
                       const void *pixels) noexcept;

  void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x,
                           GLint y, GLsizei width, GLsizei height) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glDrawRangeElements ::Krys::Gfx::OpenGL::glDrawRangeElements
  #define glTexImage3D ::Krys::Gfx::OpenGL::glTexImage3D
  #define glTexSubImage3D ::Krys::Gfx::OpenGL::glTexSubImage3D
  #define glCopyTexSubImage3D ::Krys::Gfx::OpenGL::glCopyTexSubImage3D
#endif

#pragma endregion

#pragma region 1.3
  void glActiveTexture(GLenum texture) noexcept;

  void glSampleCoverage(GLfloat value, GLboolean invert) noexcept;

  void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width,
                              GLsizei height, GLsizei depth, GLint border, GLsizei imageSize,
                              const void *data) noexcept;

  void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width,
                              GLsizei height, GLint border, GLsizei imageSize, const void *data) noexcept;

  void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border,
                              GLsizei imageSize, const void *data) noexcept;

  void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                                 GLsizei width, GLsizei height, GLsizei depth, GLenum format,
                                 GLsizei imageSize, const void *data) noexcept;

  void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                                 GLsizei height, GLenum format, GLsizei imageSize, const void *data) noexcept;

  void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format,
                                 GLsizei imageSize, const void *data) noexcept;

  void glGetCompressedTexImage(GLenum target, GLint level, void *img) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glActiveTexture ::Krys::Gfx::OpenGL::glActiveTexture
  #define glSampleCoverage ::Krys::Gfx::OpenGL::glSampleCoverage
  #define glCompressedTexImage3D ::Krys::Gfx::OpenGL::glCompressedTexImage3D
  #define glCompressedTexImage2D ::Krys::Gfx::OpenGL::glCompressedTexImage2D
  #define glCompressedTexImage1D ::Krys::Gfx::OpenGL::glCompressedTexImage1D
  #define glCompressedTexSubImage3D ::Krys::Gfx::OpenGL::glCompressedTexSubImage3D
  #define glCompressedTexSubImage2D ::Krys::Gfx::OpenGL::glCompressedTexSubImage2D
  #define glCompressedTexSubImage1D ::Krys::Gfx::OpenGL::glCompressedTexSubImage1D
  #define glGetCompressedTexImage ::Krys::Gfx::OpenGL::glGetCompressedTexImage
#endif

#pragma endregion

#pragma region 1.4

  void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha,
                           GLenum dfactorAlpha) noexcept;

  void glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount) noexcept;

  void glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices,
                           GLsizei drawcount) noexcept;

  void glPointParameterf(GLenum pname, GLfloat param) noexcept;

  void glPointParameterfv(GLenum pname, const GLfloat *params) noexcept;

  void glPointParameteri(GLenum pname, GLint param) noexcept;

  void glPointParameteriv(GLenum pname, const GLint *params) noexcept;

  void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) noexcept;

  void glBlendEquation(GLenum mode) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glBlendFuncSeparate ::Krys::Gfx::OpenGL::glBlendFuncSeparate
  #define glMultiDrawArrays ::Krys::Gfx::OpenGL::glMultiDrawArrays
  #define glMultiDrawElements ::Krys::Gfx::OpenGL::glMultiDrawElements
  #define glPointParameterf ::Krys::Gfx::OpenGL::glPointParameterf
  #define glPointParameterfv ::Krys::Gfx::OpenGL::glPointParameterfv
  #define glPointParameteri ::Krys::Gfx::OpenGL::glPointParameteri
  #define glPointParameteriv ::Krys::Gfx::OpenGL::glPointParameteriv
  #define glBlendColor ::Krys::Gfx::OpenGL::glBlendColor
  #define glBlendEquation ::Krys::Gfx::OpenGL::glBlendEquation
#endif

#pragma endregion

#pragma region 1.5

  void glGenQueries(GLsizei n, GLuint *ids) noexcept;

  void glDeleteQueries(GLsizei n, const GLuint *ids) noexcept;

  KRYS_NODISCARD GLboolean glIsQuery(GLuint id) noexcept;

  void glBeginQuery(GLenum target, GLuint id) noexcept;

  void glEndQuery(GLenum target) noexcept;

  void glGetQueryiv(GLenum target, GLenum pname, GLint *params) noexcept;

  void glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params) noexcept;

  void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params) noexcept;

  void glBindBuffer(GLenum target, GLuint buffer) noexcept;

  void glDeleteBuffers(GLsizei n, const GLuint *buffers) noexcept;

  void glGenBuffers(GLsizei n, GLuint *buffers) noexcept;

  KRYS_NODISCARD GLboolean glIsBuffer(GLuint buffer) noexcept;

  void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) noexcept;

  void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data) noexcept;

  void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void *data) noexcept;

  void *glMapBuffer(GLenum target, GLenum access) noexcept;

  KRYS_NODISCARD GLboolean glUnmapBuffer(GLenum target) noexcept;

  void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params) noexcept;

  void glGetBufferPointerv(GLenum target, GLenum pname, void **params) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glGenQueries ::Krys::Gfx::OpenGL::glGenQueries
  #define glDeleteQueries ::Krys::Gfx::OpenGL::glDeleteQueries
  #define glIsQuery ::Krys::Gfx::OpenGL::glIsQuery
  #define glBeginQuery ::Krys::Gfx::OpenGL::glBeginQuery
  #define glEndQuery ::Krys::Gfx::OpenGL::glEndQuery
  #define glGetQueryiv ::Krys::Gfx::OpenGL::glGetQueryiv
  #define glGetQueryObjectiv ::Krys::Gfx::OpenGL::glGetQueryObjectiv
  #define glGetQueryObjectuiv ::Krys::Gfx::OpenGL::glGetQueryObjectuiv
  #define glBindBuffer ::Krys::Gfx::OpenGL::glBindBuffer
  #define glDeleteBuffers ::Krys::Gfx::OpenGL::glDeleteBuffers
  #define glGenBuffers ::Krys::Gfx::OpenGL::glGenBuffers
  #define glIsBuffer ::Krys::Gfx::OpenGL::glIsBuffer
  #define glBufferData ::Krys::Gfx::OpenGL::glBufferData
  #define glBufferSubData ::Krys::Gfx::OpenGL::glBufferSubData
  #define glGetBufferSubData ::Krys::Gfx::OpenGL::glGetBufferSubData
  #define glMapBuffer ::Krys::Gfx::OpenGL::glMapBuffer
  #define glUnmapBuffer ::Krys::Gfx::OpenGL::glUnmapBuffer
  #define glGetBufferParameteriv ::Krys::Gfx::OpenGL::glGetBufferParameteriv
  #define glGetBufferPointerv ::Krys::Gfx::OpenGL::glGetBufferPointerv
#endif

#pragma endregion

#pragma region 2.0

  void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) noexcept;

  void glDrawBuffers(GLsizei n, const GLenum *bufs) noexcept;

  void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) noexcept;

  void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) noexcept;

  void glStencilMaskSeparate(GLenum face, GLuint mask) noexcept;

  void glAttachShader(GLuint program, GLuint shader) noexcept;

  void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name) noexcept;

  void glCompileShader(GLuint shader) noexcept;

  KRYS_NODISCARD GLuint glCreateProgram() noexcept;

  KRYS_NODISCARD GLuint glCreateShader(GLenum type) noexcept;

  void glDeleteProgram(GLuint program) noexcept;

  void glDeleteShader(GLuint shader) noexcept;

  void glDetachShader(GLuint program, GLuint shader) noexcept;

  void glDisableVertexAttribArray(GLuint index) noexcept;

  void glEnableVertexAttribArray(GLuint index) noexcept;

  void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size,
                         GLenum *type, GLchar *name) noexcept;

  void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size,
                          GLenum *type, GLchar *name) noexcept;

  void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders) noexcept;

  KRYS_NODISCARD GLint glGetAttribLocation(GLuint program, const GLchar *name) noexcept;

  void glGetProgramiv(GLuint program, GLenum pname, GLint *params) noexcept;

  void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) noexcept;

  void glGetShaderiv(GLuint shader, GLenum pname, GLint *params) noexcept;

  void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) noexcept;

  void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) noexcept;

  KRYS_NODISCARD GLint glGetUniformLocation(GLuint program, const GLchar *name) noexcept;

  void glGetUniformfv(GLuint program, GLint location, GLfloat *params) noexcept;

  void glGetUniformiv(GLuint program, GLint location, GLint *params) noexcept;

  void glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble *params) noexcept;

  void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params) noexcept;

  void glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params) noexcept;

  void glGetVertexAttribPointerv(GLuint index, GLenum pname, void **pointer) noexcept;

  KRYS_NODISCARD GLboolean glIsProgram(GLuint program) noexcept;

  KRYS_NODISCARD GLboolean glIsShader(GLuint shader) noexcept;

  void glLinkProgram(GLuint program) noexcept;

  void glShaderSource(GLuint shader, GLsizei count, const GLchar *const *string,
                      const GLint *length) noexcept;

  void glUseProgram(GLuint program) noexcept;

  void glUniform1f(GLint location, GLfloat v0) noexcept;

  void glUniform2f(GLint location, GLfloat v0, GLfloat v1) noexcept;

  void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) noexcept;

  void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) noexcept;

  void glUniform1i(GLint location, GLint v0) noexcept;

  void glUniform2i(GLint location, GLint v0, GLint v1) noexcept;

  void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) noexcept;

  void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) noexcept;

  void glUniform1fv(GLint location, GLsizei count, const GLfloat *value) noexcept;

  void glUniform2fv(GLint location, GLsizei count, const GLfloat *value) noexcept;

  void glUniform3fv(GLint location, GLsizei count, const GLfloat *value) noexcept;

  void glUniform4fv(GLint location, GLsizei count, const GLfloat *value) noexcept;

  void glUniform1iv(GLint location, GLsizei count, const GLint *value) noexcept;

  void glUniform2iv(GLint location, GLsizei count, const GLint *value) noexcept;

  void glUniform3iv(GLint location, GLsizei count, const GLint *value) noexcept;

  void glUniform4iv(GLint location, GLsizei count, const GLint *value) noexcept;

  void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) noexcept;

  void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) noexcept;

  void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) noexcept;

  void glValidateProgram(GLuint program) noexcept;

  void glVertexAttrib1d(GLuint index, GLdouble x) noexcept;

  void glVertexAttrib1dv(GLuint index, const GLdouble *v) noexcept;

  void glVertexAttrib1f(GLuint index, GLfloat x) noexcept;

  void glVertexAttrib1fv(GLuint index, const GLfloat *v) noexcept;

  void glVertexAttrib1s(GLuint index, GLshort x) noexcept;

  void glVertexAttrib1sv(GLuint index, const GLshort *v) noexcept;

  void glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y) noexcept;

  void glVertexAttrib2dv(GLuint index, const GLdouble *v) noexcept;

  void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y) noexcept;

  void glVertexAttrib2fv(GLuint index, const GLfloat *v) noexcept;

  void glVertexAttrib2s(GLuint index, GLshort x, GLshort y) noexcept;

  void glVertexAttrib2sv(GLuint index, const GLshort *v) noexcept;

  void glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z) noexcept;

  void glVertexAttrib3dv(GLuint index, const GLdouble *v) noexcept;

  void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) noexcept;

  void glVertexAttrib3fv(GLuint index, const GLfloat *v) noexcept;

  void glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z) noexcept;

  void glVertexAttrib3sv(GLuint index, const GLshort *v) noexcept;

  void glVertexAttrib4Nbv(GLuint index, const GLbyte *v) noexcept;

  void glVertexAttrib4Niv(GLuint index, const GLint *v) noexcept;

  void glVertexAttrib4Nsv(GLuint index, const GLshort *v) noexcept;

  void glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w) noexcept;

  void glVertexAttrib4Nubv(GLuint index, const GLubyte *v) noexcept;

  void glVertexAttrib4Nuiv(GLuint index, const GLuint *v) noexcept;

  void glVertexAttrib4Nusv(GLuint index, const GLushort *v) noexcept;

  void glVertexAttrib4bv(GLuint index, const GLbyte *v) noexcept;

  void glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) noexcept;

  void glVertexAttrib4dv(GLuint index, const GLdouble *v) noexcept;

  void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) noexcept;

  void glVertexAttrib4fv(GLuint index, const GLfloat *v) noexcept;

  void glVertexAttrib4iv(GLuint index, const GLint *v) noexcept;

  void glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w) noexcept;

  void glVertexAttrib4sv(GLuint index, const GLshort *v) noexcept;

  void glVertexAttrib4ubv(GLuint index, const GLubyte *v) noexcept;

  void glVertexAttrib4uiv(GLuint index, const GLuint *v) noexcept;

  void glVertexAttrib4usv(GLuint index, const GLushort *v) noexcept;

  void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride,
                             const void *pointer) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glBlendEquationSeparate ::Krys::Gfx::OpenGL::glBlendEquationSeparate
  #define glDrawBuffers ::Krys::Gfx::OpenGL::glDrawBuffers
  #define glStencilOpSeparate ::Krys::Gfx::OpenGL::glStencilOpSeparate
  #define glStencilFuncSeparate ::Krys::Gfx::OpenGL::glStencilFuncSeparate
  #define glStencilMaskSeparate ::Krys::Gfx::OpenGL::glStencilMaskSeparate
  #define glAttachShader ::Krys::Gfx::OpenGL::glAttachShader
  #define glBindAttribLocation ::Krys::Gfx::OpenGL::glBindAttribLocation
  #define glCompileShader ::Krys::Gfx::OpenGL::glCompileShader
  #define glCreateProgram ::Krys::Gfx::OpenGL::glCreateProgram
  #define glCreateShader ::Krys::Gfx::OpenGL::glCreateShader
  #define glDeleteProgram ::Krys::Gfx::OpenGL::glDeleteProgram
  #define glDeleteShader ::Krys::Gfx::OpenGL::glDeleteShader
  #define glDetachShader ::Krys::Gfx::OpenGL::glDetachShader
  #define glDisableVertexAttribArray ::Krys::Gfx::OpenGL::glDisableVertexAttribArray
  #define glEnableVertexAttribArray ::Krys::Gfx::OpenGL::glEnableVertexAttribArray
  #define glGetActiveAttrib ::Krys::Gfx::OpenGL::glGetActiveAttrib
  #define glGetActiveUniform ::Krys::Gfx::OpenGL::glGetActiveUniform
  #define glGetAttachedShaders ::Krys::Gfx::OpenGL::glGetAttachedShaders
  #define glGetAttribLocation ::Krys::Gfx::OpenGL::glGetAttribLocation
  #define glGetProgramiv ::Krys::Gfx::OpenGL::glGetProgramiv
  #define glGetProgramInfoLog ::Krys::Gfx::OpenGL::glGetProgramInfoLog
  #define glGetShaderiv ::Krys::Gfx::OpenGL::glGetShaderiv
  #define glGetShaderInfoLog ::Krys::Gfx::OpenGL::glGetShaderInfoLog
  #define glGetShaderSource ::Krys::Gfx::OpenGL::glGetShaderSource
  #define glGetUniformLocation ::Krys::Gfx::OpenGL::glGetUniformLocation
  #define glGetUniformfv ::Krys::Gfx::OpenGL::glGetUniformfv
  #define glGetUniformiv ::Krys::Gfx::OpenGL::glGetUniformiv
  #define glGetVertexAttribdv ::Krys::Gfx::OpenGL::glGetVertexAttribdv
  #define glGetVertexAttribfv ::Krys::Gfx::OpenGL::glGetVertexAttribfv
  #define glGetVertexAttribiv ::Krys::Gfx::OpenGL::glGetVertexAttribiv
  #define glGetVertexAttribPointerv ::Krys::Gfx::OpenGL::glGetVertexAttribPointerv
  #define glIsProgram ::Krys::Gfx::OpenGL::glIsProgram
  #define glIsShader ::Krys::Gfx::OpenGL::glIsShader
  #define glLinkProgram ::Krys::Gfx::OpenGL::glLinkProgram
  #define glShaderSource ::Krys::Gfx::OpenGL::glShaderSource
  #define glUseProgram ::Krys::Gfx::OpenGL::glUseProgram
  #define glUniform1f ::Krys::Gfx::OpenGL::glUniform1f
  #define glUniform2f ::Krys::Gfx::OpenGL::glUniform2f
  #define glUniform3f ::Krys::Gfx::OpenGL::glUniform3f
  #define glUniform4f ::Krys::Gfx::OpenGL::glUniform4f
  #define glUniform1i ::Krys::Gfx::OpenGL::glUniform1i
  #define glUniform2i ::Krys::Gfx::OpenGL::glUniform2i
  #define glUniform3i ::Krys::Gfx::OpenGL::glUniform3i
  #define glUniform4i ::Krys::Gfx::OpenGL::glUniform4i
  #define glUniform1fv ::Krys::Gfx::OpenGL::glUniform1fv
  #define glUniform2fv ::Krys::Gfx::OpenGL::glUniform2fv
  #define glUniform3fv ::Krys::Gfx::OpenGL::glUniform3fv
  #define glUniform4fv ::Krys::Gfx::OpenGL::glUniform4fv
  #define glUniform1iv ::Krys::Gfx::OpenGL::glUniform1iv
  #define glUniform2iv ::Krys::Gfx::OpenGL::glUniform2iv
  #define glUniform3iv ::Krys::Gfx::OpenGL::glUniform3iv
  #define glUniform4iv ::Krys::Gfx::OpenGL::glUniform4iv
  #define glUniformMatrix2fv ::Krys::Gfx::OpenGL::glUniformMatrix2fv
  #define glUniformMatrix3fv ::Krys::Gfx::OpenGL::glUniformMatrix3fv
  #define glUniformMatrix4fv ::Krys::Gfx::OpenGL::glUniformMatrix4fv
  #define glValidateProgram ::Krys::Gfx::OpenGL::glValidateProgram
  #define glVertexAttrib1d ::Krys::Gfx::OpenGL::glVertexAttrib1d
  #define glVertexAttrib1dv ::Krys::Gfx::OpenGL::glVertexAttrib1dv
  #define glVertexAttrib1f ::Krys::Gfx::OpenGL::glVertexAttrib1f
  #define glVertexAttrib1fv ::Krys::Gfx::OpenGL::glVertexAttrib1fv
  #define glVertexAttrib1s ::Krys::Gfx::OpenGL::glVertexAttrib1s
  #define glVertexAttrib1sv ::Krys::Gfx::OpenGL::glVertexAttrib1sv
  #define glVertexAttrib2d ::Krys::Gfx::OpenGL::glVertexAttrib2d
  #define glVertexAttrib2dv ::Krys::Gfx::OpenGL::glVertexAttrib2dv
  #define glVertexAttrib2f ::Krys::Gfx::OpenGL::glVertexAttrib2f
  #define glVertexAttrib2fv ::Krys::Gfx::OpenGL::glVertexAttrib2fv
  #define glVertexAttrib2s ::Krys::Gfx::OpenGL::glVertexAttrib2s
  #define glVertexAttrib2sv ::Krys::Gfx::OpenGL::glVertexAttrib2sv
  #define glVertexAttrib3d ::Krys::Gfx::OpenGL::glVertexAttrib3d
  #define glVertexAttrib3dv ::Krys::Gfx::OpenGL::glVertexAttrib3dv
  #define glVertexAttrib3f ::Krys::Gfx::OpenGL::glVertexAttrib3f
  #define glVertexAttrib3fv ::Krys::Gfx::OpenGL::glVertexAttrib3fv
  #define glVertexAttrib3s ::Krys::Gfx::OpenGL::glVertexAttrib3s
  #define glVertexAttrib3sv ::Krys::Gfx::OpenGL::glVertexAttrib3sv
  #define glVertexAttrib4Nbv ::Krys::Gfx::OpenGL::glVertexAttrib4Nbv
  #define glVertexAttrib4Niv ::Krys::Gfx::OpenGL::glVertexAttrib4Niv
  #define glVertexAttrib4Nsv ::Krys::Gfx::OpenGL::glVertexAttrib4Nsv
  #define glVertexAttrib4Nub ::Krys::Gfx::OpenGL::glVertexAttrib4Nub
  #define glVertexAttrib4Nubv ::Krys::Gfx::OpenGL::glVertexAttrib4Nubv
  #define glVertexAttrib4Nuiv ::Krys::Gfx::OpenGL::glVertexAttrib4Nuiv
  #define glVertexAttrib4Nusv ::Krys::Gfx::OpenGL::glVertexAttrib4Nusv
  #define glVertexAttrib4bv ::Krys::Gfx::OpenGL::glVertexAttrib4bv
  #define glVertexAttrib4d ::Krys::Gfx::OpenGL::glVertexAttrib4d
  #define glVertexAttrib4dv ::Krys::Gfx::OpenGL::glVertexAttrib4dv
  #define glVertexAttrib4f ::Krys::Gfx::OpenGL::glVertexAttrib4f
  #define glVertexAttrib4fv ::Krys::Gfx::OpenGL::glVertexAttrib4fv
  #define glVertexAttrib4iv ::Krys::Gfx::OpenGL::glVertexAttrib4iv
  #define glVertexAttrib4s ::Krys::Gfx::OpenGL::glVertexAttrib4s
  #define glVertexAttrib4sv ::Krys::Gfx::OpenGL::glVertexAttrib4sv
  #define glVertexAttrib4ubv ::Krys::Gfx::OpenGL::glVertexAttrib4ubv
  #define glVertexAttrib4uiv ::Krys::Gfx::OpenGL::glVertexAttrib4uiv
  #define glVertexAttrib4usv ::Krys::Gfx::OpenGL::glVertexAttrib4usv
  #define glVertexAttribPointer ::Krys::Gfx::OpenGL::glVertexAttribPointer
#endif

#pragma endregion

#pragma region 2.1

  void glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose,
                            const GLfloat *value) noexcept;

  void glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose,
                            const GLfloat *value) noexcept;

  void glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose,
                            const GLfloat *value) noexcept;

  void glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose,
                            const GLfloat *value) noexcept;

  void glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose,
                            const GLfloat *value) noexcept;

  void glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose,
                            const GLfloat *value) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glUniformMatrix2x3fv ::Krys::Gfx::OpenGL::glUniformMatrix2x3fv
  #define glUniformMatrix3x2fv ::Krys::Gfx::OpenGL::glUniformMatrix3x2fv
  #define glUniformMatrix2x4fv ::Krys::Gfx::OpenGL::glUniformMatrix2x4fv
  #define glUniformMatrix4x2fv ::Krys::Gfx::OpenGL::glUniformMatrix4x2fv
  #define glUniformMatrix3x4fv ::Krys::Gfx::OpenGL::glUniformMatrix3x4fv
  #define glUniformMatrix4x3fv ::Krys::Gfx::OpenGL::glUniformMatrix4x3fv
#endif

#pragma endregion

#pragma region 3.0

  void glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) noexcept;

  void glGetBooleani_v(GLenum target, GLuint index, GLboolean *data) noexcept;

  void glGetIntegeri_v(GLenum target, GLuint index, GLint *data) noexcept;

  void glEnablei(GLenum target, GLuint index) noexcept;

  void glDisablei(GLenum target, GLuint index) noexcept;

  KRYS_NODISCARD GLboolean glIsEnabledi(GLenum target, GLuint index) noexcept;

  void glBeginTransformFeedback(GLenum primitiveMode) noexcept;

  void glEndTransformFeedback() noexcept;

  void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset,
                         GLsizeiptr size) noexcept;

  void glBindBufferBase(GLenum target, GLuint index, GLuint buffer) noexcept;

  void glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar *const *varyings,
                                   GLenum bufferMode) noexcept;

  void glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length,
                                     GLsizei *size, GLenum *type, GLchar *name) noexcept;

  void glClampColor(GLenum target, GLenum clamp) noexcept;

  void glBeginConditionalRender(GLuint id, GLenum mode) noexcept;

  void glEndConditionalRender() noexcept;

  void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride,
                              const void *pointer) noexcept;

  void glGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params) noexcept;

  void glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params) noexcept;

  void glVertexAttribI1i(GLuint index, GLint x) noexcept;

  void glVertexAttribI2i(GLuint index, GLint x, GLint y) noexcept;

  void glVertexAttribI3i(GLuint index, GLint x, GLint y, GLint z) noexcept;

  void glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w) noexcept;

  void glVertexAttribI1ui(GLuint index, GLuint x) noexcept;

  void glVertexAttribI2ui(GLuint index, GLuint x, GLuint y) noexcept;

  void glVertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z) noexcept;

  void glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) noexcept;

  void glVertexAttribI1iv(GLuint index, const GLint *v) noexcept;

  void glVertexAttribI2iv(GLuint index, const GLint *v) noexcept;

  void glVertexAttribI3iv(GLuint index, const GLint *v) noexcept;

  void glVertexAttribI4iv(GLuint index, const GLint *v) noexcept;

  void glVertexAttribI1uiv(GLuint index, const GLuint *v) noexcept;

  void glVertexAttribI2uiv(GLuint index, const GLuint *v) noexcept;

  void glVertexAttribI3uiv(GLuint index, const GLuint *v) noexcept;

  void glVertexAttribI4uiv(GLuint index, const GLuint *v) noexcept;

  void glVertexAttribI4bv(GLuint index, const GLbyte *v) noexcept;

  void glVertexAttribI4sv(GLuint index, const GLshort *v) noexcept;

  void glVertexAttribI4ubv(GLuint index, const GLubyte *v) noexcept;

  void glVertexAttribI4usv(GLuint index, const GLushort *v) noexcept;

  void glGetUniformuiv(GLuint program, GLint location, GLuint *params) noexcept;

  void glBindFragDataLocation(GLuint program, GLuint color, const GLchar *name) noexcept;

  KRYS_NODISCARD GLint glGetFragDataLocation(GLuint program, const GLchar *name) noexcept;

  void glUniform1ui(GLint location, GLuint v0) noexcept;

  void glUniform2ui(GLint location, GLuint v0, GLuint v1) noexcept;

  void glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2) noexcept;

  void glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) noexcept;

  void glUniform1uiv(GLint location, GLsizei count, const GLuint *value) noexcept;

  void glUniform2uiv(GLint location, GLsizei count, const GLuint *value) noexcept;

  void glUniform3uiv(GLint location, GLsizei count, const GLuint *value) noexcept;

  void glUniform4uiv(GLint location, GLsizei count, const GLuint *value) noexcept;

  void glTexParameterIiv(GLenum target, GLenum pname, const GLint *params) noexcept;

  void glTexParameterIuiv(GLenum target, GLenum pname, const GLuint *params) noexcept;

  void glGetTexParameterIiv(GLenum target, GLenum pname, GLint *params) noexcept;

  void glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint *params) noexcept;

  void glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value) noexcept;

  void glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value) noexcept;

  void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value) noexcept;

  void glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) noexcept;

  KRYS_NODISCARD const GLubyte *glGetStringi(GLenum name, GLuint index) noexcept;

  KRYS_NODISCARD GLboolean glIsRenderbuffer(GLuint renderbuffer) noexcept;

  void glBindRenderbuffer(GLenum target, GLuint renderbuffer) noexcept;

  void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers) noexcept;

  void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers) noexcept;

  void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) noexcept;

  void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params) noexcept;

  KRYS_NODISCARD GLboolean glIsFramebuffer(GLuint framebuffer) noexcept;

  void glBindFramebuffer(GLenum target, GLuint framebuffer) noexcept;

  void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers) noexcept;

  void glGenFramebuffers(GLsizei n, GLuint *framebuffers) noexcept;

  KRYS_NODISCARD GLenum glCheckFramebufferStatus(GLenum target) noexcept;

  void glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
                              GLint level) noexcept;

  void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
                              GLint level) noexcept;

  void glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level,
                              GLint zoffset) noexcept;

  void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget,
                                 GLuint renderbuffer) noexcept;

  void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname,
                                             GLint *params) noexcept;

  void glGenerateMipmap(GLenum target) noexcept;

  void glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0,
                         GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) noexcept;

  void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                                        GLsizei height) noexcept;

  void glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level,
                                 GLint layer) noexcept;

  KRYS_NODISCARD void *glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length,
                                    GLbitfield access) noexcept;

  void glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length) noexcept;

  void glBindVertexArray(GLuint array) noexcept;

  void glDeleteVertexArrays(GLsizei n, const GLuint *arrays) noexcept;

  void glGenVertexArrays(GLsizei n, GLuint *arrays) noexcept;

  KRYS_NODISCARD GLboolean glIsVertexArray(GLuint array) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glColorMaski ::Krys::Gfx::OpenGL::glColorMaski
  #define glGetBooleani_v ::Krys::Gfx::OpenGL::glGetBooleani_v
  #define glGetIntegeri_v ::Krys::Gfx::OpenGL::glGetIntegeri_v
  #define glEnablei ::Krys::Gfx::OpenGL::glEnablei
  #define glDisablei ::Krys::Gfx::OpenGL::glDisablei
  #define glIsEnabledi ::Krys::Gfx::OpenGL::glIsEnabledi
  #define glBeginTransformFeedback ::Krys::Gfx::OpenGL::glBeginTransformFeedback
  #define glEndTransformFeedback ::Krys::Gfx::OpenGL::glEndTransformFeedback
  #define glBindBufferRange ::Krys::Gfx::OpenGL::glBindBufferRange
  #define glBindBufferBase ::Krys::Gfx::OpenGL::glBindBufferBase
  #define glTransformFeedbackVaryings ::Krys::Gfx::OpenGL::glTransformFeedbackVaryings
  #define glGetTransformFeedbackVarying ::Krys::Gfx::OpenGL::glGetTransformFeedbackVarying
  #define glClampColor ::Krys::Gfx::OpenGL::glClampColor
  #define glBeginConditionalRender ::Krys::Gfx::OpenGL::glBeginConditionalRender
  #define glEndConditionalRender ::Krys::Gfx::OpenGL::glEndConditionalRender
  #define glVertexAttribIPointer ::Krys::Gfx::OpenGL::glVertexAttribIPointer
  #define glGetVertexAttribIiv ::Krys::Gfx::OpenGL::glGetVertexAttribIiv
  #define glGetVertexAttribIuiv ::Krys::Gfx::OpenGL::glGetVertexAttribIuiv
  #define glVertexAttribI1i ::Krys::Gfx::OpenGL::glVertexAttribI1i
  #define glVertexAttribI2i ::Krys::Gfx::OpenGL::glVertexAttribI2i
  #define glVertexAttribI3i ::Krys::Gfx::OpenGL::glVertexAttribI3i
  #define glVertexAttribI4i ::Krys::Gfx::OpenGL::glVertexAttribI4i
  #define glVertexAttribI1ui ::Krys::Gfx::OpenGL::glVertexAttribI1ui
  #define glVertexAttribI2ui ::Krys::Gfx::OpenGL::glVertexAttribI2ui
  #define glVertexAttribI3ui ::Krys::Gfx::OpenGL::glVertexAttribI3ui
  #define glVertexAttribI4ui ::Krys::Gfx::OpenGL::glVertexAttribI4ui
  #define glVertexAttribI1iv ::Krys::Gfx::OpenGL::glVertexAttribI1iv
  #define glVertexAttribI2iv ::Krys::Gfx::OpenGL::glVertexAttribI2iv
  #define glVertexAttribI3iv ::Krys::Gfx::OpenGL::glVertexAttribI3iv
  #define glVertexAttribI4iv ::Krys::Gfx::OpenGL::glVertexAttribI4iv
  #define glVertexAttribI1uiv ::Krys::Gfx::OpenGL::glVertexAttribI1uiv
  #define glVertexAttribI2uiv ::Krys::Gfx::OpenGL::glVertexAttribI2uiv
  #define glVertexAttribI3uiv ::Krys::Gfx::OpenGL::glVertexAttribI3uiv
  #define glVertexAttribI4uiv ::Krys::Gfx::OpenGL::glVertexAttribI4uiv
  #define glVertexAttribI4bv ::Krys::Gfx::OpenGL::glVertexAttribI4bv
  #define glVertexAttribI4sv ::Krys::Gfx::OpenGL::glVertexAttribI4sv
  #define glVertexAttribI4ubv ::Krys::Gfx::OpenGL::glVertexAttribI4ubv
  #define glVertexAttribI4usv ::Krys::Gfx::OpenGL::glVertexAttribI4usv
  #define glGetUniformuiv ::Krys::Gfx::OpenGL::glGetUniformuiv
  #define glBindFragDataLocation ::Krys::Gfx::OpenGL::glBindFragDataLocation
  #define glGetFragDataLocation ::Krys::Gfx::OpenGL::glGetFragDataLocation
  #define glUniform1ui ::Krys::Gfx::OpenGL::glUniform1ui
  #define glUniform2ui ::Krys::Gfx::OpenGL::glUniform2ui
  #define glUniform3ui ::Krys::Gfx::OpenGL::glUniform3ui
  #define glUniform4ui ::Krys::Gfx::OpenGL::glUniform4ui
  #define glUniform1uiv ::Krys::Gfx::OpenGL::glUniform1uiv
  #define glUniform2uiv ::Krys::Gfx::OpenGL::glUniform2uiv
  #define glUniform3uiv ::Krys::Gfx::OpenGL::glUniform3uiv
  #define glUniform4uiv ::Krys::Gfx::OpenGL::glUniform4uiv
  #define glTexParameterIiv ::Krys::Gfx::OpenGL::glTexParameterIiv
  #define glTexParameterIuiv ::Krys::Gfx::OpenGL::glTexParameterIuiv
  #define glGetTexParameterIiv ::Krys::Gfx::OpenGL::glGetTexParameterIiv
  #define glGetTexParameterIuiv ::Krys::Gfx::OpenGL::glGetTexParameterIuiv
  #define glClearBufferiv ::Krys::Gfx::OpenGL::glClearBufferiv
  #define glClearBufferuiv ::Krys::Gfx::OpenGL::glClearBufferuiv
  #define glClearBufferfv ::Krys::Gfx::OpenGL::glClearBufferfv
  #define glClearBufferfi ::Krys::Gfx::OpenGL::glClearBufferfi
  #define glGetStringi ::Krys::Gfx::OpenGL::glGetStringi
  #define glIsRenderbuffer ::Krys::Gfx::OpenGL::glIsRenderbuffer
  #define glBindRenderbuffer ::Krys::Gfx::OpenGL::glBindRenderbuffer
  #define glDeleteRenderbuffers ::Krys::Gfx::OpenGL::glDeleteRenderbuffers
  #define glGenRenderbuffers ::Krys::Gfx::OpenGL::glGenRenderbuffers
  #define glRenderbufferStorage ::Krys::Gfx::OpenGL::glRenderbufferStorage
  #define glGetRenderbufferParameteriv ::Krys::Gfx::OpenGL::glGetRenderbufferParameteriv
  #define glIsFramebuffer ::Krys::Gfx::OpenGL::glIsFramebuffer
  #define glBindFramebuffer ::Krys::Gfx::OpenGL::glBindFramebuffer
  #define glDeleteFramebuffers ::Krys::Gfx::OpenGL::glDeleteFramebuffers
  #define glGenFramebuffers ::Krys::Gfx::OpenGL::glGenFramebuffers
  #define glCheckFramebufferStatus ::Krys::Gfx::OpenGL::glCheckFramebufferStatus
  #define glFramebufferTexture1D ::Krys::Gfx::OpenGL::glFramebufferTexture1D
  #define glFramebufferTexture2D ::Krys::Gfx::OpenGL::glFramebufferTexture2D
  #define glFramebufferTexture3D ::Krys::Gfx::OpenGL::glFramebufferTexture3D
  #define glFramebufferRenderbuffer ::Krys::Gfx::OpenGL::glFramebufferRenderbuffer
  #define glGetFramebufferAttachmentParameteriv ::Krys::Gfx::OpenGL::glGetFramebufferAttachmentParameteriv
  #define glGenerateMipmap ::Krys::Gfx::OpenGL::glGenerateMipmap
  #define glBlitFramebuffer ::Krys::Gfx::OpenGL::glBlitFramebuffer
  #define glRenderbufferStorageMultisample ::Krys::Gfx::OpenGL::glRenderbufferStorageMultisample
  #define glFramebufferTextureLayer ::Krys::Gfx::OpenGL::glFramebufferTextureLayer
  #define glMapBufferRange ::Krys::Gfx::OpenGL::glMapBufferRange
  #define glFlushMappedBufferRange ::Krys::Gfx::OpenGL::glFlushMappedBufferRange
  #define glBindVertexArray ::Krys::Gfx::OpenGL::glBindVertexArray
  #define glDeleteVertexArrays ::Krys::Gfx::OpenGL::glDeleteVertexArrays
  #define glGenVertexArrays ::Krys::Gfx::OpenGL::glGenVertexArrays
  #define glIsVertexArray ::Krys::Gfx::OpenGL::glIsVertexArray
#endif

#pragma endregion

#pragma region 3.1

  void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) noexcept;

  void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices,
                               GLsizei instancecount) noexcept;

  void glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer) noexcept;

  void glPrimitiveRestartIndex(GLuint index) noexcept;

  void glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset,
                           GLsizeiptr size) noexcept;

  void glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar *const *uniformNames,
                           GLuint *uniformIndices) noexcept;

  void glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname,
                             GLint *params) noexcept;

  void glGetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length,
                              GLchar *uniformName) noexcept;

  KRYS_NODISCARD GLuint glGetUniformBlockIndex(GLuint program, const GLchar *uniformBlockName) noexcept;

  void glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname,
                                 GLint *params) noexcept;

  void glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length,
                                   GLchar *uniformBlockName) noexcept;

  void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glDrawArraysInstanced ::Krys::Gfx::OpenGL::glDrawArraysInstanced
  #define glDrawElementsInstanced ::Krys::Gfx::OpenGL::glDrawElementsInstanced
  #define glTexBuffer ::Krys::Gfx::OpenGL::glTexBuffer
  #define glPrimitiveRestartIndex ::Krys::Gfx::OpenGL::glPrimitiveRestartIndex
  #define glCopyBufferSubData ::Krys::Gfx::OpenGL::glCopyBufferSubData
  #define glGetUniformIndices ::Krys::Gfx::OpenGL::glGetUniformIndices
  #define glGetActiveUniformsiv ::Krys::Gfx::OpenGL::glGetActiveUniformsiv
  #define glGetActiveUniformName ::Krys::Gfx::OpenGL::glGetActiveUniformName
  #define glGetUniformBlockIndex ::Krys::Gfx::OpenGL::glGetUniformBlockIndex
  #define glGetActiveUniformBlockiv ::Krys::Gfx::OpenGL::glGetActiveUniformBlockiv
  #define glGetActiveUniformBlockName ::Krys::Gfx::OpenGL::glGetActiveUniformBlockName
  #define glUniformBlockBinding ::Krys::Gfx::OpenGL::glUniformBlockBinding
#endif

#pragma endregion

#pragma region 3.2

  void glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices,
                                GLint basevertex) noexcept;

  void glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type,
                                     const void *indices, GLint basevertex) noexcept;

  void glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices,
                                         GLsizei instancecount, GLint basevertex) noexcept;

  void glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei *count, GLenum type,
                                     const void *const *indices, GLsizei drawcount,
                                     const GLint *basevertex) noexcept;

  void glProvokingVertex(GLenum mode) noexcept;

  KRYS_NODISCARD GLsync glFenceSync(GLenum condition, GLbitfield flags) noexcept;

  KRYS_NODISCARD GLboolean glIsSync(GLsync sync) noexcept;

  void glDeleteSync(GLsync sync) noexcept;

  KRYS_NODISCARD GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) noexcept;

  void glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) noexcept;

  void glGetInteger64v(GLenum pname, GLint64 *data) noexcept;

  void glGetSynciv(GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values) noexcept;

  void glGetInteger64i_v(GLenum target, GLuint index, GLint64 *data) noexcept;

  void glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64 *params) noexcept;

  void glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level) noexcept;

  void glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                               GLsizei height, GLboolean fixedsamplelocations) noexcept;

  void glTexImage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                               GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) noexcept;

  void glGetMultisamplefv(GLenum pname, GLuint index, GLfloat *val) noexcept;

  void glSampleMaski(GLuint maskNumber, GLbitfield mask) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glDrawElementsBaseVertex ::Krys::Gfx::OpenGL::glDrawElementsBaseVertex
  #define glDrawRangeElementsBaseVertex ::Krys::Gfx::OpenGL::glDrawRangeElementsBaseVertex
  #define glDrawElementsInstancedBaseVertex ::Krys::Gfx::OpenGL::glDrawElementsInstancedBaseVertex
  #define glMultiDrawElementsBaseVertex ::Krys::Gfx::OpenGL::glMultiDrawElementsBaseVertex
  #define glProvokingVertex ::Krys::Gfx::OpenGL::glProvokingVertex
  #define glFenceSync ::Krys::Gfx::OpenGL::glFenceSync
  #define glIsSync ::Krys::Gfx::OpenGL::glIsSync
  #define glDeleteSync ::Krys::Gfx::OpenGL::glDeleteSync
  #define glClientWaitSync ::Krys::Gfx::OpenGL::glClientWaitSync
  #define glWaitSync ::Krys::Gfx::OpenGL::glWaitSync
  #define glGetInteger64v ::Krys::Gfx::OpenGL::glGetInteger64v
  #define glGetSynciv ::Krys::Gfx::OpenGL::glGetSynciv
  #define glGetInteger64i_v ::Krys::Gfx::OpenGL::glGetInteger64i_v
  #define glGetBufferParameteri64v ::Krys::Gfx::OpenGL::glGetBufferParameteri64v
  #define glFramebufferTexture ::Krys::Gfx::OpenGL::glFramebufferTexture
  #define glTexImage2DMultisample ::Krys::Gfx::OpenGL::glTexImage2DMultisample
  #define glTexImage3DMultisample ::Krys::Gfx::OpenGL::glTexImage3DMultisample
  #define glGetMultisamplefv ::Krys::Gfx::OpenGL::glGetMultisamplefv
  #define glSampleMaski ::Krys::Gfx::OpenGL::glSampleMaski
#endif

#pragma endregion

#pragma region 3.3

  void glBindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index,
                                     const GLchar *name) noexcept;

  KRYS_NODISCARD GLint glGetFragDataIndex(GLuint program, const GLchar *name) noexcept;

  void glGenSamplers(GLsizei count, GLuint *samplers) noexcept;

  void glDeleteSamplers(GLsizei count, const GLuint *samplers) noexcept;

  KRYS_NODISCARD GLboolean glIsSampler(GLuint sampler) noexcept;

  void glBindSampler(GLuint unit, GLuint sampler) noexcept;

  void glSamplerParameteri(GLuint sampler, GLenum pname, GLint param) noexcept;

  void glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint *param) noexcept;

  void glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param) noexcept;

  void glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *param) noexcept;

  void glSamplerParameterIiv(GLuint sampler, GLenum pname, const GLint *param) noexcept;

  void glSamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint *param) noexcept;

  void glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params) noexcept;

  void glGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint *params) noexcept;

  void glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params) noexcept;

  void glGetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint *params) noexcept;

  void glQueryCounter(GLuint id, GLenum target) noexcept;

  void glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64 *params) noexcept;

  void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 *params) noexcept;

  void glVertexAttribDivisor(GLuint index, GLuint divisor) noexcept;

  void glVertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) noexcept;

  void glVertexAttribP1uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value) noexcept;

  void glVertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) noexcept;

  void glVertexAttribP2uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value) noexcept;

  void glVertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) noexcept;

  void glVertexAttribP3uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value) noexcept;

  void glVertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) noexcept;

  void glVertexAttribP4uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glBindFragDataLocationIndexed ::Krys::Gfx::OpenGL::glBindFragDataLocationIndexed
  #define glGetFragDataIndex ::Krys::Gfx::OpenGL::glGetFragDataIndex
  #define glGenSamplers ::Krys::Gfx::OpenGL::glGenSamplers
  #define glDeleteSamplers ::Krys::Gfx::OpenGL::glDeleteSamplers
  #define glIsSampler ::Krys::Gfx::OpenGL::glIsSampler
  #define glBindSampler ::Krys::Gfx::OpenGL::glBindSampler
  #define glSamplerParameteri ::Krys::Gfx::OpenGL::glSamplerParameteri
  #define glSamplerParameteriv ::Krys::Gfx::OpenGL::glSamplerParameteriv
  #define glSamplerParameterf ::Krys::Gfx::OpenGL::glSamplerParameterf
  #define glSamplerParameterfv ::Krys::Gfx::OpenGL::glSamplerParameterfv
  #define glSamplerParameterIiv ::Krys::Gfx::OpenGL::glSamplerParameterIiv
  #define glSamplerParameterIuiv ::Krys::Gfx::OpenGL::glSamplerParameterIuiv
  #define glGetSamplerParameteriv ::Krys::Gfx::OpenGL::glGetSamplerParameteriv
  #define glGetSamplerParameterIiv ::Krys::Gfx::OpenGL::glGetSamplerParameterIiv
  #define glGetSamplerParameterfv ::Krys::Gfx::OpenGL::glGetSamplerParameterfv
  #define glGetSamplerParameterIuiv ::Krys::Gfx::OpenGL::glGetSamplerParameterIuiv
  #define glQueryCounter ::Krys::Gfx::OpenGL::glQueryCounter
  #define glGetQueryObjecti64v ::Krys::Gfx::OpenGL::glGetQueryObjecti64v
  #define glGetQueryObjectui64v ::Krys::Gfx::OpenGL::glGetQueryObjectui64v
  #define glVertexAttribDivisor ::Krys::Gfx::OpenGL::glVertexAttribDivisor
  #define glVertexAttribP1ui ::Krys::Gfx::OpenGL::glVertexAttribP1ui
  #define glVertexAttribP1uiv ::Krys::Gfx::OpenGL::glVertexAttribP1uiv
  #define glVertexAttribP2ui ::Krys::Gfx::OpenGL::glVertexAttribP2ui
  #define glVertexAttribP2uiv ::Krys::Gfx::OpenGL::glVertexAttribP2uiv
  #define glVertexAttribP3ui ::Krys::Gfx::OpenGL::glVertexAttribP3ui
  #define glVertexAttribP3uiv ::Krys::Gfx::OpenGL::glVertexAttribP3uiv
  #define glVertexAttribP4ui ::Krys::Gfx::OpenGL::glVertexAttribP4ui
  #define glVertexAttribP4uiv ::Krys::Gfx::OpenGL::glVertexAttribP4uiv
#endif

#pragma endregion

#pragma region 4.0

  void glMinSampleShading(GLfloat value) noexcept;

  void glBlendEquationi(GLuint buf, GLenum mode) noexcept;

  void glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha) noexcept;

  void glBlendFunci(GLuint buf, GLenum src, GLenum dst) noexcept;

  void glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha,
                            GLenum dstAlpha) noexcept;

  void glDrawArraysIndirect(GLenum mode, const void *indirect) noexcept;

  void glDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect) noexcept;

  void glUniform1d(GLint location, GLdouble x) noexcept;

  void glUniform2d(GLint location, GLdouble x, GLdouble y) noexcept;

  void glUniform3d(GLint location, GLdouble x, GLdouble y, GLdouble z) noexcept;

  void glUniform4d(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w) noexcept;

  void glUniform1dv(GLint location, GLsizei count, const GLdouble *value) noexcept;

  void glUniform2dv(GLint location, GLsizei count, const GLdouble *value) noexcept;

  void glUniform3dv(GLint location, GLsizei count, const GLdouble *value) noexcept;

  void glUniform4dv(GLint location, GLsizei count, const GLdouble *value) noexcept;

  void glUniformMatrix2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) noexcept;

  void glUniformMatrix3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) noexcept;

  void glUniformMatrix4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value) noexcept;

  void glUniformMatrix2x3dv(GLint location, GLsizei count, GLboolean transpose,
                            const GLdouble *value) noexcept;

  void glUniformMatrix2x4dv(GLint location, GLsizei count, GLboolean transpose,
                            const GLdouble *value) noexcept;

  void glUniformMatrix3x2dv(GLint location, GLsizei count, GLboolean transpose,
                            const GLdouble *value) noexcept;

  void glUniformMatrix3x4dv(GLint location, GLsizei count, GLboolean transpose,
                            const GLdouble *value) noexcept;

  void glUniformMatrix4x2dv(GLint location, GLsizei count, GLboolean transpose,
                            const GLdouble *value) noexcept;

  void glUniformMatrix4x3dv(GLint location, GLsizei count, GLboolean transpose,
                            const GLdouble *value) noexcept;

  void glGetUniformdv(GLuint program, GLint location, GLdouble *params) noexcept;

  KRYS_NODISCARD GLint glGetSubroutineUniformLocation(GLuint program, GLenum shadertype,
                                                  const GLchar *name) noexcept;

  KRYS_NODISCARD GLuint glGetSubroutineIndex(GLuint program, GLenum shadertype, const GLchar *name) noexcept;

  void glGetActiveSubroutineUniformiv(GLuint program, GLenum shadertype, GLuint index, GLenum pname,
                                      GLint *values) noexcept;

  void glGetActiveSubroutineUniformName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize,
                                        GLsizei *length, GLchar *name) noexcept;

  void glGetActiveSubroutineName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize,
                                 GLsizei *length, GLchar *name) noexcept;

  void glUniformSubroutinesuiv(GLenum shadertype, GLsizei count, const GLuint *indices) noexcept;

  void glGetUniformSubroutineuiv(GLenum shadertype, GLint location, GLuint *params) noexcept;

  void glGetProgramStageiv(GLuint program, GLenum shadertype, GLenum pname, GLint *values) noexcept;

  void glPatchParameteri(GLenum pname, GLint value) noexcept;

  void glPatchParameterfv(GLenum pname, const GLfloat *values) noexcept;

  void glBindTransformFeedback(GLenum target, GLuint id) noexcept;

  void glDeleteTransformFeedbacks(GLsizei n, const GLuint *ids) noexcept;

  void glGenTransformFeedbacks(GLsizei n, GLuint *ids) noexcept;

  KRYS_NODISCARD GLboolean glIsTransformFeedback(GLuint id) noexcept;

  void glPauseTransformFeedback() noexcept;

  void glResumeTransformFeedback() noexcept;

  void glDrawTransformFeedback(GLenum mode, GLuint id) noexcept;

  void glDrawTransformFeedbackStream(GLenum mode, GLuint id, GLuint stream) noexcept;

  void glBeginQueryIndexed(GLenum target, GLuint index, GLuint id) noexcept;

  void glEndQueryIndexed(GLenum target, GLuint index) noexcept;

  void glGetQueryIndexediv(GLenum target, GLuint index, GLenum pname, GLint *params) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glMinSampleShading ::Krys::Gfx::OpenGL::glMinSampleShading
  #define glBlendEquationi ::Krys::Gfx::OpenGL::glBlendEquationi
  #define glBlendEquationSeparatei ::Krys::Gfx::OpenGL::glBlendEquationSeparatei
  #define glBlendFunci ::Krys::Gfx::OpenGL::glBlendFunci
  #define glBlendFuncSeparatei ::Krys::Gfx::OpenGL::glBlendFuncSeparatei
  #define glDrawArraysIndirect ::Krys::Gfx::OpenGL::glDrawArraysIndirect
  #define glDrawElementsIndirect ::Krys::Gfx::OpenGL::glDrawElementsIndirect
  #define glUniform1d ::Krys::Gfx::OpenGL::glUniform1d
  #define glUniform2d ::Krys::Gfx::OpenGL::glUniform2d
  #define glUniform3d ::Krys::Gfx::OpenGL::glUniform3d
  #define glUniform4d ::Krys::Gfx::OpenGL::glUniform4d
  #define glUniform1dv ::Krys::Gfx::OpenGL::glUniform1dv
  #define glUniform2dv ::Krys::Gfx::OpenGL::glUniform2dv
  #define glUniform3dv ::Krys::Gfx::OpenGL::glUniform3dv
  #define glUniform4dv ::Krys::Gfx::OpenGL::glUniform4dv
  #define glUniformMatrix2dv ::Krys::Gfx::OpenGL::glUniformMatrix2dv
  #define glUniformMatrix3dv ::Krys::Gfx::OpenGL::glUniformMatrix3dv
  #define glUniformMatrix4dv ::Krys::Gfx::OpenGL::glUniformMatrix4dv
  #define glUniformMatrix2x3dv ::Krys::Gfx::OpenGL::glUniformMatrix2x3dv
  #define glUniformMatrix2x4dv ::Krys::Gfx::OpenGL::glUniformMatrix2x4dv
  #define glUniformMatrix3x2dv ::Krys::Gfx::OpenGL::glUniformMatrix3x2dv
  #define glUniformMatrix3x4dv ::Krys::Gfx::OpenGL::glUniformMatrix3x4dv
  #define glUniformMatrix4x2dv ::Krys::Gfx::OpenGL::glUniformMatrix4x2dv
  #define glUniformMatrix4x3dv ::Krys::Gfx::OpenGL::glUniformMatrix4x3dv
  #define glGetUniformdv ::Krys::Gfx::OpenGL::glGetUniformdv
  #define glGetSubroutineUniformLocation ::Krys::Gfx::OpenGL::glGetSubroutineUniformLocation
  #define glGetSubroutineIndex ::Krys::Gfx::OpenGL::glGetSubroutineIndex
  #define glGetActiveSubroutineUniformiv ::Krys::Gfx::OpenGL::glGetActiveSubroutineUniformiv
  #define glGetActiveSubroutineUniformName ::Krys::Gfx::OpenGL::glGetActiveSubroutineUniformName
  #define glGetActiveSubroutineName ::Krys::Gfx::OpenGL::glGetActiveSubroutineName
  #define glUniformSubroutinesuiv ::Krys::Gfx::OpenGL::glUniformSubroutinesuiv
  #define glGetUniformSubroutineuiv ::Krys::Gfx::OpenGL::glGetUniformSubroutineuiv
  #define glGetProgramStageiv ::Krys::Gfx::OpenGL::glGetProgramStageiv
  #define glPatchParameteri ::Krys::Gfx::OpenGL::glPatchParameteri
  #define glPatchParameterfv ::Krys::Gfx::OpenGL::glPatchParameterfv
  #define glBindTransformFeedback ::Krys::Gfx::OpenGL::glBindTransformFeedback
  #define glDeleteTransformFeedbacks ::Krys::Gfx::OpenGL::glDeleteTransformFeedbacks
  #define glGenTransformFeedbacks ::Krys::Gfx::OpenGL::glGenTransformFeedbacks
  #define glIsTransformFeedback ::Krys::Gfx::OpenGL::glIsTransformFeedback
  #define glPauseTransformFeedback ::Krys::Gfx::OpenGL::glPauseTransformFeedback
  #define glResumeTransformFeedback ::Krys::Gfx::OpenGL::glResumeTransformFeedback
  #define glDrawTransformFeedback ::Krys::Gfx::OpenGL::glDrawTransformFeedback
  #define glDrawTransformFeedbackStream ::Krys::Gfx::OpenGL::glDrawTransformFeedbackStream
  #define glBeginQueryIndexed ::Krys::Gfx::OpenGL::glBeginQueryIndexed
  #define glEndQueryIndexed ::Krys::Gfx::OpenGL::glEndQueryIndexed
  #define glGetQueryIndexediv ::Krys::Gfx::OpenGL::glGetQueryIndexediv
#endif

#pragma endregion

#pragma region 4.1

  void glReleaseShaderCompiler() noexcept;

  void glShaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryFormat, const void *binary,
                      GLsizei length) noexcept;

  void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range,
                                  GLint *precision) noexcept;

  void glDepthRangef(GLfloat n, GLfloat f) noexcept;

  void glClearDepthf(GLfloat d) noexcept;

  void glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat,
                          void *binary) noexcept;

  void glProgramBinary(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length) noexcept;

  void glProgramParameteri(GLuint program, GLenum pname, GLint value) noexcept;

  void glUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program) noexcept;

  void glActiveShaderProgram(GLuint pipeline, GLuint program) noexcept;

  KRYS_NODISCARD GLuint glCreateShaderProgramv(GLenum type, GLsizei count, const GLchar *const *strings) noexcept;

  void glBindProgramPipeline(GLuint pipeline) noexcept;

  void glDeleteProgramPipelines(GLsizei n, const GLuint *pipelines) noexcept;

  void glGenProgramPipelines(GLsizei n, GLuint *pipelines) noexcept;

  KRYS_NODISCARD GLboolean glIsProgramPipeline(GLuint pipeline) noexcept;

  void glGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint *params) noexcept;

  void glProgramUniform1i(GLuint program, GLint location, GLint v0) noexcept;

  void glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint *value) noexcept;

  void glProgramUniform1f(GLuint program, GLint location, GLfloat v0) noexcept;

  void glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat *value) noexcept;

  void glProgramUniform1d(GLuint program, GLint location, GLdouble v0) noexcept;

  void glProgramUniform1dv(GLuint program, GLint location, GLsizei count, const GLdouble *value) noexcept;

  void glProgramUniform1ui(GLuint program, GLint location, GLuint v0) noexcept;

  void glProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint *value) noexcept;

  void glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1) noexcept;

  void glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint *value) noexcept;

  void glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1) noexcept;

  void glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat *value) noexcept;

  void glProgramUniform2d(GLuint program, GLint location, GLdouble v0, GLdouble v1) noexcept;

  void glProgramUniform2dv(GLuint program, GLint location, GLsizei count, const GLdouble *value) noexcept;

  void glProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1) noexcept;

  void glProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint *value) noexcept;

  void glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2) noexcept;

  void glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint *value) noexcept;

  void glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) noexcept;

  void glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat *value) noexcept;

  void glProgramUniform3d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2) noexcept;

  void glProgramUniform3dv(GLuint program, GLint location, GLsizei count, const GLdouble *value) noexcept;

  void glProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2) noexcept;

  void glProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint *value) noexcept;

  void glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) noexcept;

  void glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint *value) noexcept;

  void glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2,
                          GLfloat v3) noexcept;

  void glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat *value) noexcept;

  void glProgramUniform4d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2,
                          GLdouble v3) noexcept;

  void glProgramUniform4dv(GLuint program, GLint location, GLsizei count, const GLdouble *value) noexcept;

  void glProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2,
                           GLuint v3) noexcept;

  void glProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint *value) noexcept;

  void glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                 const GLfloat *value) noexcept;

  void glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                 const GLfloat *value) noexcept;

  void glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                 const GLfloat *value) noexcept;

  void glProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                 const GLdouble *value) noexcept;

  void glProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                 const GLdouble *value) noexcept;

  void glProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                 const GLdouble *value) noexcept;

  void glProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                   const GLfloat *value) noexcept;

  void glProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                   const GLfloat *value) noexcept;

  void glProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                   const GLfloat *value) noexcept;

  void glProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                   const GLfloat *value) noexcept;

  void glProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                   const GLfloat *value) noexcept;

  void glProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                   const GLfloat *value) noexcept;

  void glProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                   const GLdouble *value) noexcept;

  void glProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                   const GLdouble *value) noexcept;

  void glProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                   const GLdouble *value) noexcept;

  void glProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                   const GLdouble *value) noexcept;

  void glProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                   const GLdouble *value) noexcept;

  void glProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose,
                                   const GLdouble *value) noexcept;

  void glValidateProgramPipeline(GLuint pipeline) noexcept;

  void glGetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei *length,
                                   GLchar *infoLog) noexcept;

  void glVertexAttribL1d(GLuint index, GLdouble x) noexcept;

  void glVertexAttribL2d(GLuint index, GLdouble x, GLdouble y) noexcept;

  void glVertexAttribL3d(GLuint index, GLdouble x, GLdouble y, GLdouble z) noexcept;

  void glVertexAttribL4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) noexcept;

  void glVertexAttribL1dv(GLuint index, const GLdouble *v) noexcept;

  void glVertexAttribL2dv(GLuint index, const GLdouble *v) noexcept;

  void glVertexAttribL3dv(GLuint index, const GLdouble *v) noexcept;

  void glVertexAttribL4dv(GLuint index, const GLdouble *v) noexcept;

  void glVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride,
                              const void *pointer) noexcept;

  void glGetVertexAttribLdv(GLuint index, GLenum pname, GLdouble *params) noexcept;

  void glViewportArrayv(GLuint first, GLsizei count, const GLfloat *v) noexcept;

  void glViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h) noexcept;

  void glViewportIndexedfv(GLuint index, const GLfloat *v) noexcept;

  void glScissorArrayv(GLuint first, GLsizei count, const GLint *v) noexcept;

  void glScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height) noexcept;

  void glScissorIndexedv(GLuint index, const GLint *v) noexcept;

  void glDepthRangeArrayv(GLuint first, GLsizei count, const GLdouble *v) noexcept;

  void glDepthRangeIndexed(GLuint index, GLdouble n, GLdouble f) noexcept;

  void glGetFloati_v(GLenum target, GLuint index, GLfloat *data) noexcept;

  void glGetDoublei_v(GLenum target, GLuint index, GLdouble *data) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glReleaseShaderCompiler ::Krys::Gfx::OpenGL::glReleaseShaderCompiler
  #define glShaderBinary ::Krys::Gfx::OpenGL::glShaderBinary
  #define glGetShaderPrecisionFormat ::Krys::Gfx::OpenGL::glGetShaderPrecisionFormat
  #define glDepthRangef ::Krys::Gfx::OpenGL::glDepthRangef
  #define glClearDepthf ::Krys::Gfx::OpenGL::glClearDepthf
  #define glGetProgramBinary ::Krys::Gfx::OpenGL::glGetProgramBinary
  #define glProgramBinary ::Krys::Gfx::OpenGL::glProgramBinary
  #define glProgramParameteri ::Krys::Gfx::OpenGL::glProgramParameteri
  #define glUseProgramStages ::Krys::Gfx::OpenGL::glUseProgramStages
  #define glActiveShaderProgram ::Krys::Gfx::OpenGL::glActiveShaderProgram
  #define glCreateShaderProgramv ::Krys::Gfx::OpenGL::glCreateShaderProgramv
  #define glBindProgramPipeline ::Krys::Gfx::OpenGL::glBindProgramPipeline
  #define glDeleteProgramPipelines ::Krys::Gfx::OpenGL::glDeleteProgramPipelines
  #define glGenProgramPipelines ::Krys::Gfx::OpenGL::glGenProgramPipelines
  #define glIsProgramPipeline ::Krys::Gfx::OpenGL::glIsProgramPipeline
  #define glGetProgramPipelineiv ::Krys::Gfx::OpenGL::glGetProgramPipelineiv
  #define glProgramUniform1i ::Krys::Gfx::OpenGL::glProgramUniform1i
  #define glProgramUniform1iv ::Krys::Gfx::OpenGL::glProgramUniform1iv
  #define glProgramUniform1f ::Krys::Gfx::OpenGL::glProgramUniform1f
  #define glProgramUniform1fv ::Krys::Gfx::OpenGL::glProgramUniform1fv
  #define glProgramUniform1d ::Krys::Gfx::OpenGL::glProgramUniform1d
  #define glProgramUniform1dv ::Krys::Gfx::OpenGL::glProgramUniform1dv
  #define glProgramUniform1ui ::Krys::Gfx::OpenGL::glProgramUniform1ui
  #define glProgramUniform1uiv ::Krys::Gfx::OpenGL::glProgramUniform1uiv
  #define glProgramUniform2i ::Krys::Gfx::OpenGL::glProgramUniform2i
  #define glProgramUniform2iv ::Krys::Gfx::OpenGL::glProgramUniform2iv
  #define glProgramUniform2f ::Krys::Gfx::OpenGL::glProgramUniform2f
  #define glProgramUniform2fv ::Krys::Gfx::OpenGL::glProgramUniform2fv
  #define glProgramUniform2d ::Krys::Gfx::OpenGL::glProgramUniform2d
  #define glProgramUniform2dv ::Krys::Gfx::OpenGL::glProgramUniform2dv
  #define glProgramUniform2ui ::Krys::Gfx::OpenGL::glProgramUniform2ui
  #define glProgramUniform2uiv ::Krys::Gfx::OpenGL::glProgramUniform2uiv
  #define glProgramUniform3i ::Krys::Gfx::OpenGL::glProgramUniform3i
  #define glProgramUniform3iv ::Krys::Gfx::OpenGL::glProgramUniform3iv
  #define glProgramUniform3f ::Krys::Gfx::OpenGL::glProgramUniform3f
  #define glProgramUniform3fv ::Krys::Gfx::OpenGL::glProgramUniform3fv
  #define glProgramUniform3d ::Krys::Gfx::OpenGL::glProgramUniform3d
  #define glProgramUniform3dv ::Krys::Gfx::OpenGL::glProgramUniform3dv
  #define glProgramUniform3ui ::Krys::Gfx::OpenGL::glProgramUniform3ui
  #define glProgramUniform3uiv ::Krys::Gfx::OpenGL::glProgramUniform3uiv
  #define glProgramUniform4i ::Krys::Gfx::OpenGL::glProgramUniform4i
  #define glProgramUniform4iv ::Krys::Gfx::OpenGL::glProgramUniform4iv
  #define glProgramUniform4f ::Krys::Gfx::OpenGL::glProgramUniform4f
  #define glProgramUniform4fv ::Krys::Gfx::OpenGL::glProgramUniform4fv
  #define glProgramUniform4d ::Krys::Gfx::OpenGL::glProgramUniform4d
  #define glProgramUniform4dv ::Krys::Gfx::OpenGL::glProgramUniform4dv
  #define glProgramUniform4ui ::Krys::Gfx::OpenGL::glProgramUniform4ui
  #define glProgramUniform4uiv ::Krys::Gfx::OpenGL::glProgramUniform4uiv
  #define glProgramUniformMatrix2fv ::Krys::Gfx::OpenGL::glProgramUniformMatrix2fv
  #define glProgramUniformMatrix3fv ::Krys::Gfx::OpenGL::glProgramUniformMatrix3fv
  #define glProgramUniformMatrix4fv ::Krys::Gfx::OpenGL::glProgramUniformMatrix4fv
  #define glProgramUniformMatrix2dv ::Krys::Gfx::OpenGL::glProgramUniformMatrix2dv
  #define glProgramUniformMatrix3dv ::Krys::Gfx::OpenGL::glProgramUniformMatrix3dv
  #define glProgramUniformMatrix4dv ::Krys::Gfx::OpenGL::glProgramUniformMatrix4dv
  #define glProgramUniformMatrix2x3fv ::Krys::Gfx::OpenGL::glProgramUniformMatrix2x3fv
  #define glProgramUniformMatrix3x2fv ::Krys::Gfx::OpenGL::glProgramUniformMatrix3x2fv
  #define glProgramUniformMatrix2x4fv ::Krys::Gfx::OpenGL::glProgramUniformMatrix2x4fv
  #define glProgramUniformMatrix4x2fv ::Krys::Gfx::OpenGL::glProgramUniformMatrix4x2fv
  #define glProgramUniformMatrix3x4fv ::Krys::Gfx::OpenGL::glProgramUniformMatrix3x4fv
  #define glProgramUniformMatrix4x3fv ::Krys::Gfx::OpenGL::glProgramUniformMatrix4x3fv
  #define glProgramUniformMatrix2x3dv ::Krys::Gfx::OpenGL::glProgramUniformMatrix2x3dv
  #define glProgramUniformMatrix3x2dv ::Krys::Gfx::OpenGL::glProgramUniformMatrix3x2dv
  #define glProgramUniformMatrix2x4dv ::Krys::Gfx::OpenGL::glProgramUniformMatrix2x4dv
  #define glProgramUniformMatrix4x2dv ::Krys::Gfx::OpenGL::glProgramUniformMatrix4x2dv
  #define glProgramUniformMatrix3x4dv ::Krys::Gfx::OpenGL::glProgramUniformMatrix3x4dv
  #define glProgramUniformMatrix4x3dv ::Krys::Gfx::OpenGL::glProgramUniformMatrix4x3dv
  #define glValidateProgramPipeline ::Krys::Gfx::OpenGL::glValidateProgramPipeline
  #define glGetProgramPipelineInfoLog ::Krys::Gfx::OpenGL::glGetProgramPipelineInfoLog
  #define glVertexAttribL1d ::Krys::Gfx::OpenGL::glVertexAttribL1d
  #define glVertexAttribL2d ::Krys::Gfx::OpenGL::glVertexAttribL2d
  #define glVertexAttribL3d ::Krys::Gfx::OpenGL::glVertexAttribL3d
  #define glVertexAttribL4d ::Krys::Gfx::OpenGL::glVertexAttribL4d
  #define glVertexAttribL1dv ::Krys::Gfx::OpenGL::glVertexAttribL1dv
  #define glVertexAttribL2dv ::Krys::Gfx::OpenGL::glVertexAttribL2dv
  #define glVertexAttribL3dv ::Krys::Gfx::OpenGL::glVertexAttribL3dv
  #define glVertexAttribL4dv ::Krys::Gfx::OpenGL::glVertexAttribL4dv
  #define glVertexAttribLPointer ::Krys::Gfx::OpenGL::glVertexAttribLPointer
  #define glGetVertexAttribLdv ::Krys::Gfx::OpenGL::glGetVertexAttribLdv
  #define glViewportArrayv ::Krys::Gfx::OpenGL::glViewportArrayv
  #define glViewportIndexedf ::Krys::Gfx::OpenGL::glViewportIndexedf
  #define glViewportIndexedfv ::Krys::Gfx::OpenGL::glViewportIndexedfv
  #define glScissorArrayv ::Krys::Gfx::OpenGL::glScissorArrayv
  #define glScissorIndexed ::Krys::Gfx::OpenGL::glScissorIndexed
  #define glScissorIndexedv ::Krys::Gfx::OpenGL::glScissorIndexedv
  #define glDepthRangeArrayv ::Krys::Gfx::OpenGL::glDepthRangeArrayv
  #define glDepthRangeIndexed ::Krys::Gfx::OpenGL::glDepthRangeIndexed
  #define glGetFloati_v ::Krys::Gfx::OpenGL::glGetFloati_v
  #define glGetDoublei_v ::Krys::Gfx::OpenGL::glGetDoublei_v
#endif

#pragma endregion

#pragma region 4.2

  void glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount,
                                         GLuint baseinstance) noexcept;

  void glDrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void *indices,
                                           GLsizei instancecount, GLuint baseinstance) noexcept;

  void glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type,
                                                     const void *indices, GLsizei instancecount,
                                                     GLint basevertex, GLuint baseinstance) noexcept;

  void glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei count,
                             GLint *params) noexcept;

  void glGetActiveAtomicCounterBufferiv(GLuint program, GLuint bufferIndex, GLenum pname,
                                        GLint *params) noexcept;

  void glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer,
                          GLenum access, GLenum format) noexcept;

  void glMemoryBarrier(GLbitfield barriers) noexcept;

  void glTexStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width) noexcept;

  void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width,
                      GLsizei height) noexcept;

  void glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height,
                      GLsizei depth) noexcept;

  void glDrawTransformFeedbackInstanced(GLenum mode, GLuint id, GLsizei instancecount) noexcept;

  void glDrawTransformFeedbackStreamInstanced(GLenum mode, GLuint id, GLuint stream,
                                              GLsizei instancecount) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glDrawArraysInstancedBaseInstance ::Krys::Gfx::OpenGL::glDrawArraysInstancedBaseInstance
  #define glDrawElementsInstancedBaseInstance ::Krys::Gfx::OpenGL::glDrawElementsInstancedBaseInstance
  #define glDrawElementsInstancedBaseVertexBaseInstance                                                      \
    ::Krys::Gfx::OpenGL::glDrawElementsInstancedBaseVertexBaseInstance
  #define glGetInternalformativ ::Krys::Gfx::OpenGL::glGetInternalformativ
  #define glGetActiveAtomicCounterBufferiv ::Krys::Gfx::OpenGL::glGetActiveAtomicCounterBufferiv
  #define glBindImageTexture ::Krys::Gfx::OpenGL::glBindImageTexture
  #define glMemoryBarrier ::Krys::Gfx::OpenGL::glMemoryBarrier
  #define glTexStorage1D ::Krys::Gfx::OpenGL::glTexStorage1D
  #define glTexStorage2D ::Krys::Gfx::OpenGL::glTexStorage2D
  #define glTexStorage3D ::Krys::Gfx::OpenGL::glTexStorage3D
  #define glDrawTransformFeedbackInstanced ::Krys::Gfx::OpenGL::glDrawTransformFeedbackInstanced
  #define glDrawTransformFeedbackStreamInstanced ::Krys::Gfx::OpenGL::glDrawTransformFeedbackStreamInstanced
#endif

#pragma endregion

#pragma region 4.3

  void glClearBufferData(GLenum target, GLenum internalformat, GLenum format, GLenum type,
                         const void *data) noexcept;

  void glClearBufferSubData(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size,
                            GLenum format, GLenum type, const void *data) noexcept;

  void glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) noexcept;

  void glDispatchComputeIndirect(GLintptr indirect) noexcept;

  void glCopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY,
                          GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX,
                          GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight,
                          GLsizei srcDepth) noexcept;

  void glFramebufferParameteri(GLenum target, GLenum pname, GLint param) noexcept;

  void glGetFramebufferParameteriv(GLenum target, GLenum pname, GLint *params) noexcept;

  void glGetInternalformati64v(GLenum target, GLenum internalformat, GLenum pname, GLsizei count,
                               GLint64 *params) noexcept;

  void glInvalidateTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                               GLsizei width, GLsizei height, GLsizei depth) noexcept;

  void glInvalidateTexImage(GLuint texture, GLint level) noexcept;

  void glInvalidateBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr length) noexcept;

  void glInvalidateBufferData(GLuint buffer) noexcept;

  void glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments) noexcept;

  void glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x,
                                  GLint y, GLsizei width, GLsizei height) noexcept;

  void glMultiDrawArraysIndirect(GLenum mode, const void *indirect, GLsizei drawcount,
                                 GLsizei stride) noexcept;

  void glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount,
                                   GLsizei stride) noexcept;

  void glGetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint *params) noexcept;

  KRYS_NODISCARD GLuint glGetProgramResourceIndex(GLuint program, GLenum programInterface,
                                              const GLchar *name) noexcept;

  void glGetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize,
                                GLsizei *length, GLchar *name) noexcept;

  void glGetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount,
                              const GLenum *props, GLsizei count, GLsizei *length, GLint *params) noexcept;

  KRYS_NODISCARD GLint glGetProgramResourceLocation(GLuint program, GLenum programInterface,
                                                const GLchar *name) noexcept;

  KRYS_NODISCARD GLint glGetProgramResourceLocationIndex(GLuint program, GLenum programInterface,
                                                     const GLchar *name) noexcept;

  void glShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex,
                                   GLuint storageBlockBinding) noexcept;

  void glTexBufferRange(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset,
                        GLsizeiptr size) noexcept;

  void glTexStorage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                                 GLsizei height, GLboolean fixedsamplelocations) noexcept;

  void glTexStorage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width,
                                 GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) noexcept;

  void glTextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat,
                     GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers) noexcept;

  void glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) noexcept;

  void glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized,
                            GLuint relativeoffset) noexcept;

  void glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) noexcept;

  void glVertexAttribLFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) noexcept;

  void glVertexAttribBinding(GLuint attribindex, GLuint bindingindex) noexcept;

  void glVertexBindingDivisor(GLuint bindingindex, GLuint divisor) noexcept;

  void glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids,
                             GLboolean enabled) noexcept;

  void glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                            const GLchar *buf) noexcept;

  void glDebugMessageCallback(GLDEBUGPROC callback, const void *userParam) noexcept;

  KRYS_NODISCARD GLuint glGetDebugMessageLog(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types,
                                         GLuint *ids, GLenum *severities, GLsizei *lengths,
                                         GLchar *messageLog) noexcept;

  void glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar *message) noexcept;

  void glPopDebugGroup() noexcept;

  void glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar *label) noexcept;

  void glGetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length,
                        GLchar *label) noexcept;

  void glObjectPtrLabel(const void *ptr, GLsizei length, const GLchar *label) noexcept;

  void glGetObjectPtrLabel(const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glClearBufferData ::Krys::Gfx::OpenGL::glClearBufferData
  #define glClearBufferSubData ::Krys::Gfx::OpenGL::glClearBufferSubData
  #define glDispatchCompute ::Krys::Gfx::OpenGL::glDispatchCompute
  #define glDispatchComputeIndirect ::Krys::Gfx::OpenGL::glDispatchComputeIndirect
  #define glCopyImageSubData ::Krys::Gfx::OpenGL::glCopyImageSubData
  #define glFramebufferParameteri ::Krys::Gfx::OpenGL::glFramebufferParameteri
  #define glGetFramebufferParameteriv ::Krys::Gfx::OpenGL::glGetFramebufferParameteriv
  #define glGetInternalformati64v ::Krys::Gfx::OpenGL::glGetInternalformati64v
  #define glInvalidateTexSubImage ::Krys::Gfx::OpenGL::glInvalidateTexSubImage
  #define glInvalidateTexImage ::Krys::Gfx::OpenGL::glInvalidateTexImage
  #define glInvalidateBufferSubData ::Krys::Gfx::OpenGL::glInvalidateBufferSubData
  #define glInvalidateBufferData ::Krys::Gfx::OpenGL::glInvalidateBufferData
  #define glInvalidateFramebuffer ::Krys::Gfx::OpenGL::glInvalidateFramebuffer
  #define glInvalidateSubFramebuffer ::Krys::Gfx::OpenGL::glInvalidateSubFramebuffer
  #define glMultiDrawArraysIndirect ::Krys::Gfx::OpenGL::glMultiDrawArraysIndirect
  #define glMultiDrawElementsIndirect ::Krys::Gfx::OpenGL::glMultiDrawElementsIndirect
  #define glGetProgramInterfaceiv ::Krys::Gfx::OpenGL::glGetProgramInterfaceiv
  #define glGetProgramResourceIndex ::Krys::Gfx::OpenGL::glGetProgramResourceIndex
  #define glGetProgramResourceName ::Krys::Gfx::OpenGL::glGetProgramResourceName
  #define glGetProgramResourceiv ::Krys::Gfx::OpenGL::glGetProgramResourceiv
  #define glGetProgramResourceLocation ::Krys::Gfx::OpenGL::glGetProgramResourceLocation
  #define glGetProgramResourceLocationIndex ::Krys::Gfx::OpenGL::glGetProgramResourceLocationIndex
  #define glShaderStorageBlockBinding ::Krys::Gfx::OpenGL::glShaderStorageBlockBinding
  #define glTexBufferRange ::Krys::Gfx::OpenGL::glTexBufferRange
  #define glTexStorage2DMultisample ::Krys::Gfx::OpenGL::glTexStorage2DMultisample
  #define glTexStorage3DMultisample ::Krys::Gfx::OpenGL::glTexStorage3DMultisample
  #define glTextureView ::Krys::Gfx::OpenGL::glTextureView
  #define glBindVertexBuffer ::Krys::Gfx::OpenGL::glBindVertexBuffer
  #define glVertexAttribFormat ::Krys::Gfx::OpenGL::glVertexAttribFormat
  #define glVertexAttribIFormat ::Krys::Gfx::OpenGL::glVertexAttribIFormat
  #define glVertexAttribLFormat ::Krys::Gfx::OpenGL::glVertexAttribLFormat
  #define glVertexAttribBinding ::Krys::Gfx::OpenGL::glVertexAttribBinding
  #define glVertexBindingDivisor ::Krys::Gfx::OpenGL::glVertexBindingDivisor
  #define glDebugMessageControl ::Krys::Gfx::OpenGL::glDebugMessageControl
  #define glDebugMessageInsert ::Krys::Gfx::OpenGL::glDebugMessageInsert
  #define glDebugMessageCallback ::Krys::Gfx::OpenGL::glDebugMessageCallback
  #define glGetDebugMessageLog ::Krys::Gfx::OpenGL::glGetDebugMessageLog
  #define glPushDebugGroup ::Krys::Gfx::OpenGL::glPushDebugGroup
  #define glPopDebugGroup ::Krys::Gfx::OpenGL::glPopDebugGroup
  #define glObjectLabel ::Krys::Gfx::OpenGL::glObjectLabel
  #define glGetObjectLabel ::Krys::Gfx::OpenGL::glGetObjectLabel
  #define glObjectPtrLabel ::Krys::Gfx::OpenGL::glObjectPtrLabel
  #define glGetObjectPtrLabel ::Krys::Gfx::OpenGL::glGetObjectPtrLabel
#endif

#pragma endregion

#pragma region 4.4

  void glBufferStorage(GLenum target, GLsizeiptr size, const void *data, GLbitfield flags) noexcept;

  void glClearTexImage(GLuint texture, GLint level, GLenum format, GLenum type, const void *data) noexcept;

  void glClearTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                          GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type,
                          const void *data) noexcept;

  void glBindBuffersBase(GLenum target, GLuint first, GLsizei count, const GLuint *buffers) noexcept;

  void glBindBuffersRange(GLenum target, GLuint first, GLsizei count, const GLuint *buffers,
                          const GLintptr *offsets, const GLsizeiptr *sizes) noexcept;

  void glBindTextures(GLuint first, GLsizei count, const GLuint *textures) noexcept;

  void glBindSamplers(GLuint first, GLsizei count, const GLuint *samplers) noexcept;

  void glBindImageTextures(GLuint first, GLsizei count, const GLuint *textures) noexcept;

  void glBindVertexBuffers(GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets,
                           const GLsizei *strides) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glBufferStorage ::Krys::Gfx::OpenGL::glBufferStorage
  #define glClearTexImage ::Krys::Gfx::OpenGL::glClearTexImage
  #define glClearTexSubImage ::Krys::Gfx::OpenGL::glClearTexSubImage
  #define glBindBuffersBase ::Krys::Gfx::OpenGL::glBindBuffersBase
  #define glBindBuffersRange ::Krys::Gfx::OpenGL::glBindBuffersRange
  #define glBindTextures ::Krys::Gfx::OpenGL::glBindTextures
  #define glBindSamplers ::Krys::Gfx::OpenGL::glBindSamplers
  #define glBindImageTextures ::Krys::Gfx::OpenGL::glBindImageTextures
  #define glBindVertexBuffers ::Krys::Gfx::OpenGL::glBindVertexBuffers
#endif

#pragma endregion

#pragma region 4.5

  void glClipControl(GLenum origin, GLenum depth) noexcept;

  void glCreateTransformFeedbacks(GLsizei n, GLuint *ids) noexcept;

  void glTransformFeedbackBufferBase(GLuint xfb, GLuint index, GLuint buffer) noexcept;

  void glTransformFeedbackBufferRange(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset,
                                      GLsizeiptr size) noexcept;

  void glGetTransformFeedbackiv(GLuint xfb, GLenum pname, GLint *param) noexcept;

  void glGetTransformFeedbacki_v(GLuint xfb, GLenum pname, GLuint index, GLint *param) noexcept;

  void glGetTransformFeedbacki64_v(GLuint xfb, GLenum pname, GLuint index, GLint64 *param) noexcept;

  void glCreateBuffers(GLsizei n, GLuint *buffers) noexcept;

  void glNamedBufferStorage(GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags) noexcept;

  void glNamedBufferData(GLuint buffer, GLsizeiptr size, const void *data, GLenum usage) noexcept;

  void glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data) noexcept;

  void glCopyNamedBufferSubData(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset,
                                GLintptr writeOffset, GLsizeiptr size) noexcept;

  void glClearNamedBufferData(GLuint buffer, GLenum internalformat, GLenum format, GLenum type,
                              const void *data) noexcept;

  void glClearNamedBufferSubData(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size,
                                 GLenum format, GLenum type, const void *data) noexcept;

  KRYS_NODISCARD void *glMapNamedBuffer(GLuint buffer, GLenum access) noexcept;

  KRYS_NODISCARD void *glMapNamedBufferRange(GLuint buffer, GLintptr offset, GLsizeiptr length,
                                         GLbitfield access) noexcept;

  GLboolean glUnmapNamedBuffer(GLuint buffer) noexcept;

  void glFlushMappedNamedBufferRange(GLuint buffer, GLintptr offset, GLsizeiptr length) noexcept;

  void glGetNamedBufferParameteriv(GLuint buffer, GLenum pname, GLint *params) noexcept;

  void glGetNamedBufferParameteri64v(GLuint buffer, GLenum pname, GLint64 *params) noexcept;

  void glGetNamedBufferPointerv(GLuint buffer, GLenum pname, void **params) noexcept;

  void glGetNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, void *data) noexcept;

  void glCreateFramebuffers(GLsizei n, GLuint *framebuffers) noexcept;

  void glNamedFramebufferRenderbuffer(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget,
                                      GLuint renderbuffer) noexcept;

  void glNamedFramebufferParameteri(GLuint framebuffer, GLenum pname, GLint param) noexcept;

  void glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level) noexcept;

  void glNamedFramebufferTextureLayer(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level,
                                      GLint layer) noexcept;

  void glNamedFramebufferDrawBuffer(GLuint framebuffer, GLenum buf) noexcept;

  void glNamedFramebufferDrawBuffers(GLuint framebuffer, GLsizei n, const GLenum *bufs) noexcept;

  void glNamedFramebufferReadBuffer(GLuint framebuffer, GLenum src) noexcept;

  void glInvalidateNamedFramebufferData(GLuint framebuffer, GLsizei numAttachments,
                                        const GLenum *attachments) noexcept;

  void glInvalidateNamedFramebufferSubData(GLuint framebuffer, GLsizei numAttachments,
                                           const GLenum *attachments, GLint x, GLint y, GLsizei width,
                                           GLsizei height) noexcept;

  void glClearNamedFramebufferiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer,
                                 const GLint *value) noexcept;

  void glClearNamedFramebufferuiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer,
                                  const GLuint *value) noexcept;

  void glClearNamedFramebufferfv(GLuint framebuffer, GLenum buffer, GLint drawbuffer,
                                 const GLfloat *value) noexcept;

  void glClearNamedFramebufferfi(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth,
                                 GLint stencil) noexcept;

  void glBlitNamedFramebuffer(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0,
                              GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                              GLbitfield mask, GLenum filter) noexcept;

  KRYS_NODISCARD GLenum glCheckNamedFramebufferStatus(GLuint framebuffer, GLenum target) noexcept;

  void glGetNamedFramebufferParameteriv(GLuint framebuffer, GLenum pname, GLint *param) noexcept;

  void glGetNamedFramebufferAttachmentParameteriv(GLuint framebuffer, GLenum attachment, GLenum pname,
                                                  GLint *params) noexcept;

  void glCreateRenderbuffers(GLsizei n, GLuint *renderbuffers) noexcept;

  void glNamedRenderbufferStorage(GLuint renderbuffer, GLenum internalformat, GLsizei width,
                                  GLsizei height) noexcept;

  void glNamedRenderbufferStorageMultisample(GLuint renderbuffer, GLsizei samples, GLenum internalformat,
                                             GLsizei width, GLsizei height) noexcept;

  void glGetNamedRenderbufferParameteriv(GLuint renderbuffer, GLenum pname, GLint *params) noexcept;

  void glCreateTextures(GLenum target, GLsizei n, GLuint *textures) noexcept;

  void glTextureBuffer(GLuint texture, GLenum internalformat, GLuint buffer) noexcept;

  void glTextureBufferRange(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset,
                            GLsizeiptr size) noexcept;

  void glTextureStorage1D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width) noexcept;

  void glTextureStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width,
                          GLsizei height) noexcept;

  void glTextureStorage3D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width,
                          GLsizei height, GLsizei depth) noexcept;

  void glTextureStorage2DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width,
                                     GLsizei height, GLboolean fixedsamplelocations) noexcept;

  void glTextureStorage3DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width,
                                     GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) noexcept;

  void glTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format,
                           GLenum type, const void *pixels) noexcept;

  void glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                           GLsizei height, GLenum format, GLenum type, const void *pixels) noexcept;

  void glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                           GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type,
                           const void *pixels) noexcept;

  void glCompressedTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format,
                                     GLsizei imageSize, const void *data) noexcept;

  void glCompressedTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                                     GLsizei height, GLenum format, GLsizei imageSize,
                                     const void *data) noexcept;

  void glCompressedTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                                     GLsizei width, GLsizei height, GLsizei depth, GLenum format,
                                     GLsizei imageSize, const void *data) noexcept;

  void glCopyTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y,
                               GLsizei width) noexcept;

  void glCopyTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
                               GLsizei width, GLsizei height) noexcept;

  void glCopyTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                               GLint x, GLint y, GLsizei width, GLsizei height) noexcept;

  void glTextureParameterf(GLuint texture, GLenum pname, GLfloat param) noexcept;

  void glTextureParameterfv(GLuint texture, GLenum pname, const GLfloat *param) noexcept;

  void glTextureParameteri(GLuint texture, GLenum pname, GLint param) noexcept;

  void glTextureParameterIiv(GLuint texture, GLenum pname, const GLint *params) noexcept;

  void glTextureParameterIuiv(GLuint texture, GLenum pname, const GLuint *params) noexcept;

  void glTextureParameteriv(GLuint texture, GLenum pname, const GLint *param) noexcept;

  void glGenerateTextureMipmap(GLuint texture) noexcept;

  void glBindTextureUnit(GLuint unit, GLuint texture) noexcept;

  void glGetTextureImage(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize,
                         void *pixels) noexcept;

  void glGetCompressedTextureImage(GLuint texture, GLint level, GLsizei bufSize, void *pixels) noexcept;

  void glGetTextureLevelParameterfv(GLuint texture, GLint level, GLenum pname, GLfloat *params) noexcept;

  void glGetTextureLevelParameteriv(GLuint texture, GLint level, GLenum pname, GLint *params) noexcept;

  void glGetTextureParameterfv(GLuint texture, GLenum pname, GLfloat *params) noexcept;

  void glGetTextureParameterIiv(GLuint texture, GLenum pname, GLint *params) noexcept;

  void glGetTextureParameterIuiv(GLuint texture, GLenum pname, GLuint *params) noexcept;

  void glGetTextureParameteriv(GLuint texture, GLenum pname, GLint *params) noexcept;

  void glCreateVertexArrays(GLsizei n, GLuint *arrays) noexcept;

  void glDisableVertexArrayAttrib(GLuint vaobj, GLuint index) noexcept;

  void glEnableVertexArrayAttrib(GLuint vaobj, GLuint index) noexcept;

  void glVertexArrayElementBuffer(GLuint vaobj, GLuint buffer) noexcept;

  void glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset,
                                 GLsizei stride) noexcept;

  void glVertexArrayVertexBuffers(GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers,
                                  const GLintptr *offsets, const GLsizei *strides) noexcept;

  void glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex, GLuint bindingindex) noexcept;

  void glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type,
                                 GLboolean normalized, GLuint relativeoffset) noexcept;

  void glVertexArrayAttribIFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type,
                                  GLuint relativeoffset) noexcept;

  void glVertexArrayAttribLFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type,
                                  GLuint relativeoffset) noexcept;

  void glVertexArrayBindingDivisor(GLuint vaobj, GLuint bindingindex, GLuint divisor) noexcept;

  void glGetVertexArrayiv(GLuint vaobj, GLenum pname, GLint *param) noexcept;

  void glGetVertexArrayIndexediv(GLuint vaobj, GLuint index, GLenum pname, GLint *param) noexcept;

  void glGetVertexArrayIndexed64iv(GLuint vaobj, GLuint index, GLenum pname, GLint64 *param) noexcept;

  void glCreateSamplers(GLsizei n, GLuint *samplers) noexcept;

  void glCreateProgramPipelines(GLsizei n, GLuint *pipelines) noexcept;

  void glCreateQueries(GLenum target, GLsizei n, GLuint *ids) noexcept;

  void glGetQueryBufferObjecti64v(GLuint id, GLuint buffer, GLenum pname, GLintptr offset) noexcept;

  void glGetQueryBufferObjectiv(GLuint id, GLuint buffer, GLenum pname, GLintptr offset) noexcept;

  void glGetQueryBufferObjectui64v(GLuint id, GLuint buffer, GLenum pname, GLintptr offset) noexcept;

  void glGetQueryBufferObjectuiv(GLuint id, GLuint buffer, GLenum pname, GLintptr offset) noexcept;

  void glMemoryBarrierByRegion(GLbitfield barriers) noexcept;

  void glGetTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
                            GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type,
                            GLsizei bufSize, void *pixels) noexcept;

  void glGetCompressedTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset,
                                      GLint zoffset, GLsizei width, GLsizei height, GLsizei depth,
                                      GLsizei bufSize, void *pixels) noexcept;

  KRYS_NODISCARD GLenum glGetGraphicsResetStatus() noexcept;

  void glGetnCompressedTexImage(GLenum target, GLint lod, GLsizei bufSize, void *pixels) noexcept;

  void glGetnTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize,
                      void *pixels) noexcept;

  void glGetnUniformdv(GLuint program, GLint location, GLsizei bufSize, GLdouble *params) noexcept;

  void glGetnUniformfv(GLuint program, GLint location, GLsizei bufSize, GLfloat *params) noexcept;

  void glGetnUniformiv(GLuint program, GLint location, GLsizei bufSize, GLint *params) noexcept;

  void glGetnUniformuiv(GLuint program, GLint location, GLsizei bufSize, GLuint *params) noexcept;

  void glReadnPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
                     GLsizei bufSize, void *data) noexcept;

  void glTextureBarrier() noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glClipControl ::Krys::Gfx::OpenGL::glClipControl
  #define glCreateTransformFeedbacks ::Krys::Gfx::OpenGL::glCreateTransformFeedbacks
  #define glTransformFeedbackBufferBase ::Krys::Gfx::OpenGL::glTransformFeedbackBufferBase
  #define glTransformFeedbackBufferRange ::Krys::Gfx::OpenGL::glTransformFeedbackBufferRange
  #define glGetTransformFeedbackiv ::Krys::Gfx::OpenGL::glGetTransformFeedbackiv
  #define glGetTransformFeedbacki_v ::Krys::Gfx::OpenGL::glGetTransformFeedbacki_v
  #define glGetTransformFeedbacki64_v ::Krys::Gfx::OpenGL::glGetTransformFeedbacki64_v
  #define glCreateBuffers ::Krys::Gfx::OpenGL::glCreateBuffers
  #define glNamedBufferStorage ::Krys::Gfx::OpenGL::glNamedBufferStorage
  #define glNamedBufferData ::Krys::Gfx::OpenGL::glNamedBufferData
  #define glNamedBufferSubData ::Krys::Gfx::OpenGL::glNamedBufferSubData
  #define glCopyNamedBufferSubData ::Krys::Gfx::OpenGL::glCopyNamedBufferSubData
  #define glClearNamedBufferData ::Krys::Gfx::OpenGL::glClearNamedBufferData
  #define glClearNamedBufferSubData ::Krys::Gfx::OpenGL::glClearNamedBufferSubData
  #define glMapNamedBuffer ::Krys::Gfx::OpenGL::glMapNamedBuffer
  #define glMapNamedBufferRange ::Krys::Gfx::OpenGL::glMapNamedBufferRange
  #define glUnmapNamedBuffer ::Krys::Gfx::OpenGL::glUnmapNamedBuffer
  #define glFlushMappedNamedBufferRange ::Krys::Gfx::OpenGL::glFlushMappedNamedBufferRange
  #define glGetNamedBufferParameteriv ::Krys::Gfx::OpenGL::glGetNamedBufferParameteriv
  #define glGetNamedBufferParameteri64v ::Krys::Gfx::OpenGL::glGetNamedBufferParameteri64v
  #define glGetNamedBufferPointerv ::Krys::Gfx::OpenGL::glGetNamedBufferPointerv
  #define glGetNamedBufferSubData ::Krys::Gfx::OpenGL::glGetNamedBufferSubData
  #define glCreateFramebuffers ::Krys::Gfx::OpenGL::glCreateFramebuffers
  #define glNamedFramebufferRenderbuffer ::Krys::Gfx::OpenGL::glNamedFramebufferRenderbuffer
  #define glNamedFramebufferParameteri ::Krys::Gfx::OpenGL::glNamedFramebufferParameteri
  #define glNamedFramebufferTexture ::Krys::Gfx::OpenGL::glNamedFramebufferTexture
  #define glNamedFramebufferTextureLayer ::Krys::Gfx::OpenGL::glNamedFramebufferTextureLayer
  #define glNamedFramebufferDrawBuffer ::Krys::Gfx::OpenGL::glNamedFramebufferDrawBuffer
  #define glNamedFramebufferDrawBuffers ::Krys::Gfx::OpenGL::glNamedFramebufferDrawBuffers
  #define glNamedFramebufferReadBuffer ::Krys::Gfx::OpenGL::glNamedFramebufferReadBuffer
  #define glInvalidateNamedFramebufferData ::Krys::Gfx::OpenGL::glInvalidateNamedFramebufferData
  #define glInvalidateNamedFramebufferSubData ::Krys::Gfx::OpenGL::glInvalidateNamedFramebufferSubData
  #define glClearNamedFramebufferiv ::Krys::Gfx::OpenGL::glClearNamedFramebufferiv
  #define glClearNamedFramebufferuiv ::Krys::Gfx::OpenGL::glClearNamedFramebufferuiv
  #define glClearNamedFramebufferfv ::Krys::Gfx::OpenGL::glClearNamedFramebufferfv
  #define glClearNamedFramebufferfi ::Krys::Gfx::OpenGL::glClearNamedFramebufferfi
  #define glBlitNamedFramebuffer ::Krys::Gfx::OpenGL::glBlitNamedFramebuffer
  #define glCheckNamedFramebufferStatus ::Krys::Gfx::OpenGL::glCheckNamedFramebufferStatus
  #define glGetNamedFramebufferParameteriv ::Krys::Gfx::OpenGL::glGetNamedFramebufferParameteriv
  #define glGetNamedFramebufferAttachmentParameteriv                                                         \
    ::Krys::Gfx::OpenGL::glGetNamedFramebufferAttachmentParameteriv
  #define glCreateRenderbuffers ::Krys::Gfx::OpenGL::glCreateRenderbuffers
  #define glNamedRenderbufferStorage ::Krys::Gfx::OpenGL::glNamedRenderbufferStorage
  #define glNamedRenderbufferStorageMultisample ::Krys::Gfx::OpenGL::glNamedRenderbufferStorageMultisample
  #define glGetNamedRenderbufferParameteriv ::Krys::Gfx::OpenGL::glGetNamedRenderbufferParameteriv
  #define glCreateTextures ::Krys::Gfx::OpenGL::glCreateTextures
  #define glTextureBuffer ::Krys::Gfx::OpenGL::glTextureBuffer
  #define glTextureBufferRange ::Krys::Gfx::OpenGL::glTextureBufferRange
  #define glTextureStorage1D ::Krys::Gfx::OpenGL::glTextureStorage1D
  #define glTextureStorage2D ::Krys::Gfx::OpenGL::glTextureStorage2D
  #define glTextureStorage3D ::Krys::Gfx::OpenGL::glTextureStorage3D
  #define glTextureStorage2DMultisample ::Krys::Gfx::OpenGL::glTextureStorage2DMultisample
  #define glTextureStorage3DMultisample ::Krys::Gfx::OpenGL::glTextureStorage3DMultisample
  #define glTextureSubImage1D ::Krys::Gfx::OpenGL::glTextureSubImage1D
  #define glTextureSubImage2D ::Krys::Gfx::OpenGL::glTextureSubImage2D
  #define glTextureSubImage3D ::Krys::Gfx::OpenGL::glTextureSubImage3D
  #define glCompressedTextureSubImage1D ::Krys::Gfx::OpenGL::glCompressedTextureSubImage1D
  #define glCompressedTextureSubImage2D ::Krys::Gfx::OpenGL::glCompressedTextureSubImage2D
  #define glCompressedTextureSubImage3D ::Krys::Gfx::OpenGL::glCompressedTextureSubImage3D
  #define glCopyTextureSubImage1D ::Krys::Gfx::OpenGL::glCopyTextureSubImage1D
  #define glCopyTextureSubImage2D ::Krys::Gfx::OpenGL::glCopyTextureSubImage2D
  #define glCopyTextureSubImage3D ::Krys::Gfx::OpenGL::glCopyTextureSubImage3D
  #define glTextureParameterf ::Krys::Gfx::OpenGL::glTextureParameterf
  #define glTextureParameterfv ::Krys::Gfx::OpenGL::glTextureParameterfv
  #define glTextureParameteri ::Krys::Gfx::OpenGL::glTextureParameteri
  #define glTextureParameterIiv ::Krys::Gfx::OpenGL::glTextureParameterIiv
  #define glTextureParameterIuiv ::Krys::Gfx::OpenGL::glTextureParameterIuiv
  #define glTextureParameteriv ::Krys::Gfx::OpenGL::glTextureParameteriv
  #define glGenerateTextureMipmap ::Krys::Gfx::OpenGL::glGenerateTextureMipmap
  #define glBindTextureUnit ::Krys::Gfx::OpenGL::glBindTextureUnit
  #define glGetTextureImage ::Krys::Gfx::OpenGL::glGetTextureImage
  #define glGetCompressedTextureImage ::Krys::Gfx::OpenGL::glGetCompressedTextureImage
  #define glGetTextureLevelParameterfv ::Krys::Gfx::OpenGL::glGetTextureLevelParameterfv
  #define glGetTextureLevelParameteriv ::Krys::Gfx::OpenGL::glGetTextureLevelParameteriv
  #define glGetTextureParameterfv ::Krys::Gfx::OpenGL::glGetTextureParameterfv
  #define glGetTextureParameterIiv ::Krys::Gfx::OpenGL::glGetTextureParameterIiv
  #define glGetTextureParameterIuiv ::Krys::Gfx::OpenGL::glGetTextureParameterIuiv
  #define glGetTextureParameteriv ::Krys::Gfx::OpenGL::glGetTextureParameteriv
  #define glCreateVertexArrays ::Krys::Gfx::OpenGL::glCreateVertexArrays
  #define glDisableVertexArrayAttrib ::Krys::Gfx::OpenGL::glDisableVertexArrayAttrib
  #define glEnableVertexArrayAttrib ::Krys::Gfx::OpenGL::glEnableVertexArrayAttrib
  #define glVertexArrayElementBuffer ::Krys::Gfx::OpenGL::glVertexArrayElementBuffer
  #define glVertexArrayVertexBuffer ::Krys::Gfx::OpenGL::glVertexArrayVertexBuffer
  #define glVertexArrayVertexBuffers ::Krys::Gfx::OpenGL::glVertexArrayVertexBuffers
  #define glVertexArrayAttribBinding ::Krys::Gfx::OpenGL::glVertexArrayAttribBinding
  #define glVertexArrayAttribFormat ::Krys::Gfx::OpenGL::glVertexArrayAttribFormat
  #define glVertexArrayAttribIFormat ::Krys::Gfx::OpenGL::glVertexArrayAttribIFormat
  #define glVertexArrayAttribLFormat ::Krys::Gfx::OpenGL::glVertexArrayAttribLFormat
  #define glVertexArrayBindingDivisor ::Krys::Gfx::OpenGL::glVertexArrayBindingDivisor
  #define glGetVertexArrayiv ::Krys::Gfx::OpenGL::glGetVertexArrayiv
  #define glGetVertexArrayIndexediv ::Krys::Gfx::OpenGL::glGetVertexArrayIndexediv
  #define glGetVertexArrayIndexed64iv ::Krys::Gfx::OpenGL::glGetVertexArrayIndexed64iv
  #define glCreateSamplers ::Krys::Gfx::OpenGL::glCreateSamplers
  #define glCreateProgramPipelines ::Krys::Gfx::OpenGL::glCreateProgramPipelines
  #define glCreateQueries ::Krys::Gfx::OpenGL::glCreateQueries
  #define glGetQueryBufferObjecti64v ::Krys::Gfx::OpenGL::glGetQueryBufferObjecti64v
  #define glGetQueryBufferObjectiv ::Krys::Gfx::OpenGL::glGetQueryBufferObjectiv
  #define glGetQueryBufferObjectui64v ::Krys::Gfx::OpenGL::glGetQueryBufferObjectui64v
  #define glGetQueryBufferObjectuiv ::Krys::Gfx::OpenGL::glGetQueryBufferObjectuiv
  #define glMemoryBarrierByRegion ::Krys::Gfx::OpenGL::glMemoryBarrierByRegion
  #define glGetTextureSubImage ::Krys::Gfx::OpenGL::glGetTextureSubImage
  #define glGetCompressedTextureSubImage ::Krys::Gfx::OpenGL::glGetCompressedTextureSubImage
  #define glGetGraphicsResetStatus ::Krys::Gfx::OpenGL::glGetGraphicsResetStatus
  #define glGetnCompressedTexImage ::Krys::Gfx::OpenGL::glGetnCompressedTexImage
  #define glGetnTexImage ::Krys::Gfx::OpenGL::glGetnTexImage
  #define glGetnUniformdv ::Krys::Gfx::OpenGL::glGetnUniformdv
  #define glGetnUniformfv ::Krys::Gfx::OpenGL::glGetnUniformfv
  #define glGetnUniformiv ::Krys::Gfx::OpenGL::glGetnUniformiv
  #define glGetnUniformuiv ::Krys::Gfx::OpenGL::glGetnUniformuiv
  #define glReadnPixels ::Krys::Gfx::OpenGL::glReadnPixels
  #define glTextureBarrier ::Krys::Gfx::OpenGL::glTextureBarrier
#endif

#pragma endregion

#pragma region 4.6

  void glSpecializeShader(GLuint shader, const GLchar *pEntryPoint, GLuint numSpecializationConstants,
                          const GLuint *pConstantIndex, const GLuint *pConstantValue) noexcept;

  void glMultiDrawArraysIndirectCount(GLenum mode, const void *indirect, GLintptr drawcount,
                                      GLsizei maxdrawcount, GLsizei stride) noexcept;

  void glMultiDrawElementsIndirectCount(GLenum mode, GLenum type, const void *indirect, GLintptr drawcount,
                                        GLsizei maxdrawcount, GLsizei stride) noexcept;

  void glPolygonOffsetClamp(GLfloat factor, GLfloat units, GLfloat clamp) noexcept;

#if KRYS_GL_OVERRIDE_FUNCTIONS
  #define glSpecializeShader Krystal::Gfx::OpenGL::Hooks::glSpecializeShader
  #define glMultiDrawArraysIndirectCount Krystal::Gfx::OpenGL::Hooks::glMultiDrawArraysIndirectCount
  #define glMultiDrawElementsIndirectCount Krystal::Gfx::OpenGL::Hooks::glMultiDrawElementsIndirectCount
  #define glPolygonOffsetClamp Krystal::Gfx::OpenGL::Hooks::glPolygonOffsetClamp
#endif

#pragma endregion

#pragma endregion
}