#pragma once

#include "Krystal.Core/Attributes.hpp"

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

namespace krys::Gfx::OpenGL
{
  /// @brief Initializes OpenGL hooks.
  KRYS_NODISCARD bool InitialiseGLHooks() noexcept;

  /// @brief Checks if a specific OpenGL extension is supported.
  KRYS_NODISCARD bool IsGLExtensionSupported(const char *extension) noexcept;
}

// OpenGL hook declarations
namespace krys::Gfx::OpenGL
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
  #define glCullFace ::krys::Gfx::OpenGL::glCullFace
  #define glFrontFace ::krys::Gfx::OpenGL::glFrontFace
  #define glHint ::krys::Gfx::OpenGL::glHint
  #define glLineWidth ::krys::Gfx::OpenGL::glLineWidth
  #define glPointSize ::krys::Gfx::OpenGL::glPointSize
  #define glPolygonMode ::krys::Gfx::OpenGL::glPolygonMode
  #define glScissor ::krys::Gfx::OpenGL::glScissor
  #define glTexParameterf ::krys::Gfx::OpenGL::glTexParameterf
  #define glTexParameterfv ::krys::Gfx::OpenGL::glTexParameterfv
  #define glTexParameteri ::krys::Gfx::OpenGL::glTexParameteri
  #define glTexParameteriv ::krys::Gfx::OpenGL::glTexParameteriv
  #define glTexImage1D ::krys::Gfx::OpenGL::glTexImage1D
  #define glTexImage2D ::krys::Gfx::OpenGL::glTexImage2D
  #define glDrawBuffer ::krys::Gfx::OpenGL::glDrawBuffer
  #define glClear ::krys::Gfx::OpenGL::glClear
  #define glClearColor ::krys::Gfx::OpenGL::glClearColor
  #define glClearStencil ::krys::Gfx::OpenGL::glClearStencil
  #define glClearDepth ::krys::Gfx::OpenGL::glClearDepth
  #define glStencilMask ::krys::Gfx::OpenGL::glStencilMask
  #define glColorMask ::krys::Gfx::OpenGL::glColorMask
  #define glDepthMask ::krys::Gfx::OpenGL::glDepthMask
  #define glDisable ::krys::Gfx::OpenGL::glDisable
  #define glEnable ::krys::Gfx::OpenGL::glEnable
  #define glFinish ::krys::Gfx::OpenGL::glFinish
  #define glFlush ::krys::Gfx::OpenGL::glFlush
  #define glBlendFunc ::krys::Gfx::OpenGL::glBlendFunc
  #define glLogicOp ::krys::Gfx::OpenGL::glLogicOp
  #define glStencilFunc ::krys::Gfx::OpenGL::glStencilFunc
  #define glStencilOp ::krys::Gfx::OpenGL::glStencilOp
  #define glDepthFunc ::krys::Gfx::OpenGL::glDepthFunc
  #define glPixelStoref ::krys::Gfx::OpenGL::glPixelStoref
  #define glPixelStorei ::krys::Gfx::OpenGL::glPixelStorei
  #define glReadBuffer ::krys::Gfx::OpenGL::glReadBuffer
  #define glReadPixels ::krys::Gfx::OpenGL::glReadPixels
  #define glGetBooleanv ::krys::Gfx::OpenGL::glGetBooleanv
  #define glGetDoublev ::krys::Gfx::OpenGL::glGetDoublev
  #define glGetError ::krys::Gfx::OpenGL::glGetError
  #define glGetFloatv ::krys::Gfx::OpenGL::glGetFloatv
  #define glGetIntegerv ::krys::Gfx::OpenGL::glGetIntegerv
  #define glGetString ::krys::Gfx::OpenGL::glGetString
  #define glGetTexImage ::krys::Gfx::OpenGL::glGetTexImage
  #define glGetTexParameterfv ::krys::Gfx::OpenGL::glGetTexParameterfv
  #define glGetTexParameteriv ::krys::Gfx::OpenGL::glGetTexParameteriv
  #define glGetTexLevelParameterfv ::krys::Gfx::OpenGL::glGetTexLevelParameterfv
  #define glGetTexLevelParameteriv ::krys::Gfx::OpenGL::glGetTexLevelParameteriv
  #define glIsEnabled ::krys::Gfx::OpenGL::glIsEnabled
  #define glDepthRange ::krys::Gfx::OpenGL::glDepthRange
  #define glViewport ::krys::Gfx::OpenGL::glViewport
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
  #define glDrawArrays ::krys::Gfx::OpenGL::glDrawArrays
  #define glDrawElements ::krys::Gfx::OpenGL::glDrawElements
  #define glGetPointerv ::krys::Gfx::OpenGL::glGetPointerv
  #define glPolygonOffset ::krys::Gfx::OpenGL::glPolygonOffset
  #define glCopyTexImage1D ::krys::Gfx::OpenGL::glCopyTexImage1D
  #define glCopyTexImage2D ::krys::Gfx::OpenGL::glCopyTexImage2D
  #define glCopyTexSubImage1D ::krys::Gfx::OpenGL::glCopyTexSubImage1D
  #define glCopyTexSubImage2D ::krys::Gfx::OpenGL::glCopyTexSubImage2D
  #define glTexSubImage1D ::krys::Gfx::OpenGL::glTexSubImage1D
  #define glTexSubImage2D ::krys::Gfx::OpenGL::glTexSubImage2D
  #define glBindTexture ::krys::Gfx::OpenGL::glBindTexture
  #define glDeleteTextures ::krys::Gfx::OpenGL::glDeleteTextures
  #define glGenTextures ::krys::Gfx::OpenGL::glGenTextures
  #define glIsTexture ::krys::Gfx::OpenGL::glIsTexture
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
  #define glDrawRangeElements ::krys::Gfx::OpenGL::glDrawRangeElements
  #define glTexImage3D ::krys::Gfx::OpenGL::glTexImage3D
  #define glTexSubImage3D ::krys::Gfx::OpenGL::glTexSubImage3D
  #define glCopyTexSubImage3D ::krys::Gfx::OpenGL::glCopyTexSubImage3D
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
  #define glActiveTexture ::krys::Gfx::OpenGL::glActiveTexture
  #define glSampleCoverage ::krys::Gfx::OpenGL::glSampleCoverage
  #define glCompressedTexImage3D ::krys::Gfx::OpenGL::glCompressedTexImage3D
  #define glCompressedTexImage2D ::krys::Gfx::OpenGL::glCompressedTexImage2D
  #define glCompressedTexImage1D ::krys::Gfx::OpenGL::glCompressedTexImage1D
  #define glCompressedTexSubImage3D ::krys::Gfx::OpenGL::glCompressedTexSubImage3D
  #define glCompressedTexSubImage2D ::krys::Gfx::OpenGL::glCompressedTexSubImage2D
  #define glCompressedTexSubImage1D ::krys::Gfx::OpenGL::glCompressedTexSubImage1D
  #define glGetCompressedTexImage ::krys::Gfx::OpenGL::glGetCompressedTexImage
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
  #define glBlendFuncSeparate ::krys::Gfx::OpenGL::glBlendFuncSeparate
  #define glMultiDrawArrays ::krys::Gfx::OpenGL::glMultiDrawArrays
  #define glMultiDrawElements ::krys::Gfx::OpenGL::glMultiDrawElements
  #define glPointParameterf ::krys::Gfx::OpenGL::glPointParameterf
  #define glPointParameterfv ::krys::Gfx::OpenGL::glPointParameterfv
  #define glPointParameteri ::krys::Gfx::OpenGL::glPointParameteri
  #define glPointParameteriv ::krys::Gfx::OpenGL::glPointParameteriv
  #define glBlendColor ::krys::Gfx::OpenGL::glBlendColor
  #define glBlendEquation ::krys::Gfx::OpenGL::glBlendEquation
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
  #define glGenQueries ::krys::Gfx::OpenGL::glGenQueries
  #define glDeleteQueries ::krys::Gfx::OpenGL::glDeleteQueries
  #define glIsQuery ::krys::Gfx::OpenGL::glIsQuery
  #define glBeginQuery ::krys::Gfx::OpenGL::glBeginQuery
  #define glEndQuery ::krys::Gfx::OpenGL::glEndQuery
  #define glGetQueryiv ::krys::Gfx::OpenGL::glGetQueryiv
  #define glGetQueryObjectiv ::krys::Gfx::OpenGL::glGetQueryObjectiv
  #define glGetQueryObjectuiv ::krys::Gfx::OpenGL::glGetQueryObjectuiv
  #define glBindBuffer ::krys::Gfx::OpenGL::glBindBuffer
  #define glDeleteBuffers ::krys::Gfx::OpenGL::glDeleteBuffers
  #define glGenBuffers ::krys::Gfx::OpenGL::glGenBuffers
  #define glIsBuffer ::krys::Gfx::OpenGL::glIsBuffer
  #define glBufferData ::krys::Gfx::OpenGL::glBufferData
  #define glBufferSubData ::krys::Gfx::OpenGL::glBufferSubData
  #define glGetBufferSubData ::krys::Gfx::OpenGL::glGetBufferSubData
  #define glMapBuffer ::krys::Gfx::OpenGL::glMapBuffer
  #define glUnmapBuffer ::krys::Gfx::OpenGL::glUnmapBuffer
  #define glGetBufferParameteriv ::krys::Gfx::OpenGL::glGetBufferParameteriv
  #define glGetBufferPointerv ::krys::Gfx::OpenGL::glGetBufferPointerv
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
  #define glBlendEquationSeparate ::krys::Gfx::OpenGL::glBlendEquationSeparate
  #define glDrawBuffers ::krys::Gfx::OpenGL::glDrawBuffers
  #define glStencilOpSeparate ::krys::Gfx::OpenGL::glStencilOpSeparate
  #define glStencilFuncSeparate ::krys::Gfx::OpenGL::glStencilFuncSeparate
  #define glStencilMaskSeparate ::krys::Gfx::OpenGL::glStencilMaskSeparate
  #define glAttachShader ::krys::Gfx::OpenGL::glAttachShader
  #define glBindAttribLocation ::krys::Gfx::OpenGL::glBindAttribLocation
  #define glCompileShader ::krys::Gfx::OpenGL::glCompileShader
  #define glCreateProgram ::krys::Gfx::OpenGL::glCreateProgram
  #define glCreateShader ::krys::Gfx::OpenGL::glCreateShader
  #define glDeleteProgram ::krys::Gfx::OpenGL::glDeleteProgram
  #define glDeleteShader ::krys::Gfx::OpenGL::glDeleteShader
  #define glDetachShader ::krys::Gfx::OpenGL::glDetachShader
  #define glDisableVertexAttribArray ::krys::Gfx::OpenGL::glDisableVertexAttribArray
  #define glEnableVertexAttribArray ::krys::Gfx::OpenGL::glEnableVertexAttribArray
  #define glGetActiveAttrib ::krys::Gfx::OpenGL::glGetActiveAttrib
  #define glGetActiveUniform ::krys::Gfx::OpenGL::glGetActiveUniform
  #define glGetAttachedShaders ::krys::Gfx::OpenGL::glGetAttachedShaders
  #define glGetAttribLocation ::krys::Gfx::OpenGL::glGetAttribLocation
  #define glGetProgramiv ::krys::Gfx::OpenGL::glGetProgramiv
  #define glGetProgramInfoLog ::krys::Gfx::OpenGL::glGetProgramInfoLog
  #define glGetShaderiv ::krys::Gfx::OpenGL::glGetShaderiv
  #define glGetShaderInfoLog ::krys::Gfx::OpenGL::glGetShaderInfoLog
  #define glGetShaderSource ::krys::Gfx::OpenGL::glGetShaderSource
  #define glGetUniformLocation ::krys::Gfx::OpenGL::glGetUniformLocation
  #define glGetUniformfv ::krys::Gfx::OpenGL::glGetUniformfv
  #define glGetUniformiv ::krys::Gfx::OpenGL::glGetUniformiv
  #define glGetVertexAttribdv ::krys::Gfx::OpenGL::glGetVertexAttribdv
  #define glGetVertexAttribfv ::krys::Gfx::OpenGL::glGetVertexAttribfv
  #define glGetVertexAttribiv ::krys::Gfx::OpenGL::glGetVertexAttribiv
  #define glGetVertexAttribPointerv ::krys::Gfx::OpenGL::glGetVertexAttribPointerv
  #define glIsProgram ::krys::Gfx::OpenGL::glIsProgram
  #define glIsShader ::krys::Gfx::OpenGL::glIsShader
  #define glLinkProgram ::krys::Gfx::OpenGL::glLinkProgram
  #define glShaderSource ::krys::Gfx::OpenGL::glShaderSource
  #define glUseProgram ::krys::Gfx::OpenGL::glUseProgram
  #define glUniform1f ::krys::Gfx::OpenGL::glUniform1f
  #define glUniform2f ::krys::Gfx::OpenGL::glUniform2f
  #define glUniform3f ::krys::Gfx::OpenGL::glUniform3f
  #define glUniform4f ::krys::Gfx::OpenGL::glUniform4f
  #define glUniform1i ::krys::Gfx::OpenGL::glUniform1i
  #define glUniform2i ::krys::Gfx::OpenGL::glUniform2i
  #define glUniform3i ::krys::Gfx::OpenGL::glUniform3i
  #define glUniform4i ::krys::Gfx::OpenGL::glUniform4i
  #define glUniform1fv ::krys::Gfx::OpenGL::glUniform1fv
  #define glUniform2fv ::krys::Gfx::OpenGL::glUniform2fv
  #define glUniform3fv ::krys::Gfx::OpenGL::glUniform3fv
  #define glUniform4fv ::krys::Gfx::OpenGL::glUniform4fv
  #define glUniform1iv ::krys::Gfx::OpenGL::glUniform1iv
  #define glUniform2iv ::krys::Gfx::OpenGL::glUniform2iv
  #define glUniform3iv ::krys::Gfx::OpenGL::glUniform3iv
  #define glUniform4iv ::krys::Gfx::OpenGL::glUniform4iv
  #define glUniformMatrix2fv ::krys::Gfx::OpenGL::glUniformMatrix2fv
  #define glUniformMatrix3fv ::krys::Gfx::OpenGL::glUniformMatrix3fv
  #define glUniformMatrix4fv ::krys::Gfx::OpenGL::glUniformMatrix4fv
  #define glValidateProgram ::krys::Gfx::OpenGL::glValidateProgram
  #define glVertexAttrib1d ::krys::Gfx::OpenGL::glVertexAttrib1d
  #define glVertexAttrib1dv ::krys::Gfx::OpenGL::glVertexAttrib1dv
  #define glVertexAttrib1f ::krys::Gfx::OpenGL::glVertexAttrib1f
  #define glVertexAttrib1fv ::krys::Gfx::OpenGL::glVertexAttrib1fv
  #define glVertexAttrib1s ::krys::Gfx::OpenGL::glVertexAttrib1s
  #define glVertexAttrib1sv ::krys::Gfx::OpenGL::glVertexAttrib1sv
  #define glVertexAttrib2d ::krys::Gfx::OpenGL::glVertexAttrib2d
  #define glVertexAttrib2dv ::krys::Gfx::OpenGL::glVertexAttrib2dv
  #define glVertexAttrib2f ::krys::Gfx::OpenGL::glVertexAttrib2f
  #define glVertexAttrib2fv ::krys::Gfx::OpenGL::glVertexAttrib2fv
  #define glVertexAttrib2s ::krys::Gfx::OpenGL::glVertexAttrib2s
  #define glVertexAttrib2sv ::krys::Gfx::OpenGL::glVertexAttrib2sv
  #define glVertexAttrib3d ::krys::Gfx::OpenGL::glVertexAttrib3d
  #define glVertexAttrib3dv ::krys::Gfx::OpenGL::glVertexAttrib3dv
  #define glVertexAttrib3f ::krys::Gfx::OpenGL::glVertexAttrib3f
  #define glVertexAttrib3fv ::krys::Gfx::OpenGL::glVertexAttrib3fv
  #define glVertexAttrib3s ::krys::Gfx::OpenGL::glVertexAttrib3s
  #define glVertexAttrib3sv ::krys::Gfx::OpenGL::glVertexAttrib3sv
  #define glVertexAttrib4Nbv ::krys::Gfx::OpenGL::glVertexAttrib4Nbv
  #define glVertexAttrib4Niv ::krys::Gfx::OpenGL::glVertexAttrib4Niv
  #define glVertexAttrib4Nsv ::krys::Gfx::OpenGL::glVertexAttrib4Nsv
  #define glVertexAttrib4Nub ::krys::Gfx::OpenGL::glVertexAttrib4Nub
  #define glVertexAttrib4Nubv ::krys::Gfx::OpenGL::glVertexAttrib4Nubv
  #define glVertexAttrib4Nuiv ::krys::Gfx::OpenGL::glVertexAttrib4Nuiv
  #define glVertexAttrib4Nusv ::krys::Gfx::OpenGL::glVertexAttrib4Nusv
  #define glVertexAttrib4bv ::krys::Gfx::OpenGL::glVertexAttrib4bv
  #define glVertexAttrib4d ::krys::Gfx::OpenGL::glVertexAttrib4d
  #define glVertexAttrib4dv ::krys::Gfx::OpenGL::glVertexAttrib4dv
  #define glVertexAttrib4f ::krys::Gfx::OpenGL::glVertexAttrib4f
  #define glVertexAttrib4fv ::krys::Gfx::OpenGL::glVertexAttrib4fv
  #define glVertexAttrib4iv ::krys::Gfx::OpenGL::glVertexAttrib4iv
  #define glVertexAttrib4s ::krys::Gfx::OpenGL::glVertexAttrib4s
  #define glVertexAttrib4sv ::krys::Gfx::OpenGL::glVertexAttrib4sv
  #define glVertexAttrib4ubv ::krys::Gfx::OpenGL::glVertexAttrib4ubv
  #define glVertexAttrib4uiv ::krys::Gfx::OpenGL::glVertexAttrib4uiv
  #define glVertexAttrib4usv ::krys::Gfx::OpenGL::glVertexAttrib4usv
  #define glVertexAttribPointer ::krys::Gfx::OpenGL::glVertexAttribPointer
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
  #define glUniformMatrix2x3fv ::krys::Gfx::OpenGL::glUniformMatrix2x3fv
  #define glUniformMatrix3x2fv ::krys::Gfx::OpenGL::glUniformMatrix3x2fv
  #define glUniformMatrix2x4fv ::krys::Gfx::OpenGL::glUniformMatrix2x4fv
  #define glUniformMatrix4x2fv ::krys::Gfx::OpenGL::glUniformMatrix4x2fv
  #define glUniformMatrix3x4fv ::krys::Gfx::OpenGL::glUniformMatrix3x4fv
  #define glUniformMatrix4x3fv ::krys::Gfx::OpenGL::glUniformMatrix4x3fv
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
  #define glColorMaski ::krys::Gfx::OpenGL::glColorMaski
  #define glGetBooleani_v ::krys::Gfx::OpenGL::glGetBooleani_v
  #define glGetIntegeri_v ::krys::Gfx::OpenGL::glGetIntegeri_v
  #define glEnablei ::krys::Gfx::OpenGL::glEnablei
  #define glDisablei ::krys::Gfx::OpenGL::glDisablei
  #define glIsEnabledi ::krys::Gfx::OpenGL::glIsEnabledi
  #define glBeginTransformFeedback ::krys::Gfx::OpenGL::glBeginTransformFeedback
  #define glEndTransformFeedback ::krys::Gfx::OpenGL::glEndTransformFeedback
  #define glBindBufferRange ::krys::Gfx::OpenGL::glBindBufferRange
  #define glBindBufferBase ::krys::Gfx::OpenGL::glBindBufferBase
  #define glTransformFeedbackVaryings ::krys::Gfx::OpenGL::glTransformFeedbackVaryings
  #define glGetTransformFeedbackVarying ::krys::Gfx::OpenGL::glGetTransformFeedbackVarying
  #define glClampColor ::krys::Gfx::OpenGL::glClampColor
  #define glBeginConditionalRender ::krys::Gfx::OpenGL::glBeginConditionalRender
  #define glEndConditionalRender ::krys::Gfx::OpenGL::glEndConditionalRender
  #define glVertexAttribIPointer ::krys::Gfx::OpenGL::glVertexAttribIPointer
  #define glGetVertexAttribIiv ::krys::Gfx::OpenGL::glGetVertexAttribIiv
  #define glGetVertexAttribIuiv ::krys::Gfx::OpenGL::glGetVertexAttribIuiv
  #define glVertexAttribI1i ::krys::Gfx::OpenGL::glVertexAttribI1i
  #define glVertexAttribI2i ::krys::Gfx::OpenGL::glVertexAttribI2i
  #define glVertexAttribI3i ::krys::Gfx::OpenGL::glVertexAttribI3i
  #define glVertexAttribI4i ::krys::Gfx::OpenGL::glVertexAttribI4i
  #define glVertexAttribI1ui ::krys::Gfx::OpenGL::glVertexAttribI1ui
  #define glVertexAttribI2ui ::krys::Gfx::OpenGL::glVertexAttribI2ui
  #define glVertexAttribI3ui ::krys::Gfx::OpenGL::glVertexAttribI3ui
  #define glVertexAttribI4ui ::krys::Gfx::OpenGL::glVertexAttribI4ui
  #define glVertexAttribI1iv ::krys::Gfx::OpenGL::glVertexAttribI1iv
  #define glVertexAttribI2iv ::krys::Gfx::OpenGL::glVertexAttribI2iv
  #define glVertexAttribI3iv ::krys::Gfx::OpenGL::glVertexAttribI3iv
  #define glVertexAttribI4iv ::krys::Gfx::OpenGL::glVertexAttribI4iv
  #define glVertexAttribI1uiv ::krys::Gfx::OpenGL::glVertexAttribI1uiv
  #define glVertexAttribI2uiv ::krys::Gfx::OpenGL::glVertexAttribI2uiv
  #define glVertexAttribI3uiv ::krys::Gfx::OpenGL::glVertexAttribI3uiv
  #define glVertexAttribI4uiv ::krys::Gfx::OpenGL::glVertexAttribI4uiv
  #define glVertexAttribI4bv ::krys::Gfx::OpenGL::glVertexAttribI4bv
  #define glVertexAttribI4sv ::krys::Gfx::OpenGL::glVertexAttribI4sv
  #define glVertexAttribI4ubv ::krys::Gfx::OpenGL::glVertexAttribI4ubv
  #define glVertexAttribI4usv ::krys::Gfx::OpenGL::glVertexAttribI4usv
  #define glGetUniformuiv ::krys::Gfx::OpenGL::glGetUniformuiv
  #define glBindFragDataLocation ::krys::Gfx::OpenGL::glBindFragDataLocation
  #define glGetFragDataLocation ::krys::Gfx::OpenGL::glGetFragDataLocation
  #define glUniform1ui ::krys::Gfx::OpenGL::glUniform1ui
  #define glUniform2ui ::krys::Gfx::OpenGL::glUniform2ui
  #define glUniform3ui ::krys::Gfx::OpenGL::glUniform3ui
  #define glUniform4ui ::krys::Gfx::OpenGL::glUniform4ui
  #define glUniform1uiv ::krys::Gfx::OpenGL::glUniform1uiv
  #define glUniform2uiv ::krys::Gfx::OpenGL::glUniform2uiv
  #define glUniform3uiv ::krys::Gfx::OpenGL::glUniform3uiv
  #define glUniform4uiv ::krys::Gfx::OpenGL::glUniform4uiv
  #define glTexParameterIiv ::krys::Gfx::OpenGL::glTexParameterIiv
  #define glTexParameterIuiv ::krys::Gfx::OpenGL::glTexParameterIuiv
  #define glGetTexParameterIiv ::krys::Gfx::OpenGL::glGetTexParameterIiv
  #define glGetTexParameterIuiv ::krys::Gfx::OpenGL::glGetTexParameterIuiv
  #define glClearBufferiv ::krys::Gfx::OpenGL::glClearBufferiv
  #define glClearBufferuiv ::krys::Gfx::OpenGL::glClearBufferuiv
  #define glClearBufferfv ::krys::Gfx::OpenGL::glClearBufferfv
  #define glClearBufferfi ::krys::Gfx::OpenGL::glClearBufferfi
  #define glGetStringi ::krys::Gfx::OpenGL::glGetStringi
  #define glIsRenderbuffer ::krys::Gfx::OpenGL::glIsRenderbuffer
  #define glBindRenderbuffer ::krys::Gfx::OpenGL::glBindRenderbuffer
  #define glDeleteRenderbuffers ::krys::Gfx::OpenGL::glDeleteRenderbuffers
  #define glGenRenderbuffers ::krys::Gfx::OpenGL::glGenRenderbuffers
  #define glRenderbufferStorage ::krys::Gfx::OpenGL::glRenderbufferStorage
  #define glGetRenderbufferParameteriv ::krys::Gfx::OpenGL::glGetRenderbufferParameteriv
  #define glIsFramebuffer ::krys::Gfx::OpenGL::glIsFramebuffer
  #define glBindFramebuffer ::krys::Gfx::OpenGL::glBindFramebuffer
  #define glDeleteFramebuffers ::krys::Gfx::OpenGL::glDeleteFramebuffers
  #define glGenFramebuffers ::krys::Gfx::OpenGL::glGenFramebuffers
  #define glCheckFramebufferStatus ::krys::Gfx::OpenGL::glCheckFramebufferStatus
  #define glFramebufferTexture1D ::krys::Gfx::OpenGL::glFramebufferTexture1D
  #define glFramebufferTexture2D ::krys::Gfx::OpenGL::glFramebufferTexture2D
  #define glFramebufferTexture3D ::krys::Gfx::OpenGL::glFramebufferTexture3D
  #define glFramebufferRenderbuffer ::krys::Gfx::OpenGL::glFramebufferRenderbuffer
  #define glGetFramebufferAttachmentParameteriv ::krys::Gfx::OpenGL::glGetFramebufferAttachmentParameteriv
  #define glGenerateMipmap ::krys::Gfx::OpenGL::glGenerateMipmap
  #define glBlitFramebuffer ::krys::Gfx::OpenGL::glBlitFramebuffer
  #define glRenderbufferStorageMultisample ::krys::Gfx::OpenGL::glRenderbufferStorageMultisample
  #define glFramebufferTextureLayer ::krys::Gfx::OpenGL::glFramebufferTextureLayer
  #define glMapBufferRange ::krys::Gfx::OpenGL::glMapBufferRange
  #define glFlushMappedBufferRange ::krys::Gfx::OpenGL::glFlushMappedBufferRange
  #define glBindVertexArray ::krys::Gfx::OpenGL::glBindVertexArray
  #define glDeleteVertexArrays ::krys::Gfx::OpenGL::glDeleteVertexArrays
  #define glGenVertexArrays ::krys::Gfx::OpenGL::glGenVertexArrays
  #define glIsVertexArray ::krys::Gfx::OpenGL::glIsVertexArray
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
  #define glDrawArraysInstanced ::krys::Gfx::OpenGL::glDrawArraysInstanced
  #define glDrawElementsInstanced ::krys::Gfx::OpenGL::glDrawElementsInstanced
  #define glTexBuffer ::krys::Gfx::OpenGL::glTexBuffer
  #define glPrimitiveRestartIndex ::krys::Gfx::OpenGL::glPrimitiveRestartIndex
  #define glCopyBufferSubData ::krys::Gfx::OpenGL::glCopyBufferSubData
  #define glGetUniformIndices ::krys::Gfx::OpenGL::glGetUniformIndices
  #define glGetActiveUniformsiv ::krys::Gfx::OpenGL::glGetActiveUniformsiv
  #define glGetActiveUniformName ::krys::Gfx::OpenGL::glGetActiveUniformName
  #define glGetUniformBlockIndex ::krys::Gfx::OpenGL::glGetUniformBlockIndex
  #define glGetActiveUniformBlockiv ::krys::Gfx::OpenGL::glGetActiveUniformBlockiv
  #define glGetActiveUniformBlockName ::krys::Gfx::OpenGL::glGetActiveUniformBlockName
  #define glUniformBlockBinding ::krys::Gfx::OpenGL::glUniformBlockBinding
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
  #define glDrawElementsBaseVertex ::krys::Gfx::OpenGL::glDrawElementsBaseVertex
  #define glDrawRangeElementsBaseVertex ::krys::Gfx::OpenGL::glDrawRangeElementsBaseVertex
  #define glDrawElementsInstancedBaseVertex ::krys::Gfx::OpenGL::glDrawElementsInstancedBaseVertex
  #define glMultiDrawElementsBaseVertex ::krys::Gfx::OpenGL::glMultiDrawElementsBaseVertex
  #define glProvokingVertex ::krys::Gfx::OpenGL::glProvokingVertex
  #define glFenceSync ::krys::Gfx::OpenGL::glFenceSync
  #define glIsSync ::krys::Gfx::OpenGL::glIsSync
  #define glDeleteSync ::krys::Gfx::OpenGL::glDeleteSync
  #define glClientWaitSync ::krys::Gfx::OpenGL::glClientWaitSync
  #define glWaitSync ::krys::Gfx::OpenGL::glWaitSync
  #define glGetInteger64v ::krys::Gfx::OpenGL::glGetInteger64v
  #define glGetSynciv ::krys::Gfx::OpenGL::glGetSynciv
  #define glGetInteger64i_v ::krys::Gfx::OpenGL::glGetInteger64i_v
  #define glGetBufferParameteri64v ::krys::Gfx::OpenGL::glGetBufferParameteri64v
  #define glFramebufferTexture ::krys::Gfx::OpenGL::glFramebufferTexture
  #define glTexImage2DMultisample ::krys::Gfx::OpenGL::glTexImage2DMultisample
  #define glTexImage3DMultisample ::krys::Gfx::OpenGL::glTexImage3DMultisample
  #define glGetMultisamplefv ::krys::Gfx::OpenGL::glGetMultisamplefv
  #define glSampleMaski ::krys::Gfx::OpenGL::glSampleMaski
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
  #define glBindFragDataLocationIndexed ::krys::Gfx::OpenGL::glBindFragDataLocationIndexed
  #define glGetFragDataIndex ::krys::Gfx::OpenGL::glGetFragDataIndex
  #define glGenSamplers ::krys::Gfx::OpenGL::glGenSamplers
  #define glDeleteSamplers ::krys::Gfx::OpenGL::glDeleteSamplers
  #define glIsSampler ::krys::Gfx::OpenGL::glIsSampler
  #define glBindSampler ::krys::Gfx::OpenGL::glBindSampler
  #define glSamplerParameteri ::krys::Gfx::OpenGL::glSamplerParameteri
  #define glSamplerParameteriv ::krys::Gfx::OpenGL::glSamplerParameteriv
  #define glSamplerParameterf ::krys::Gfx::OpenGL::glSamplerParameterf
  #define glSamplerParameterfv ::krys::Gfx::OpenGL::glSamplerParameterfv
  #define glSamplerParameterIiv ::krys::Gfx::OpenGL::glSamplerParameterIiv
  #define glSamplerParameterIuiv ::krys::Gfx::OpenGL::glSamplerParameterIuiv
  #define glGetSamplerParameteriv ::krys::Gfx::OpenGL::glGetSamplerParameteriv
  #define glGetSamplerParameterIiv ::krys::Gfx::OpenGL::glGetSamplerParameterIiv
  #define glGetSamplerParameterfv ::krys::Gfx::OpenGL::glGetSamplerParameterfv
  #define glGetSamplerParameterIuiv ::krys::Gfx::OpenGL::glGetSamplerParameterIuiv
  #define glQueryCounter ::krys::Gfx::OpenGL::glQueryCounter
  #define glGetQueryObjecti64v ::krys::Gfx::OpenGL::glGetQueryObjecti64v
  #define glGetQueryObjectui64v ::krys::Gfx::OpenGL::glGetQueryObjectui64v
  #define glVertexAttribDivisor ::krys::Gfx::OpenGL::glVertexAttribDivisor
  #define glVertexAttribP1ui ::krys::Gfx::OpenGL::glVertexAttribP1ui
  #define glVertexAttribP1uiv ::krys::Gfx::OpenGL::glVertexAttribP1uiv
  #define glVertexAttribP2ui ::krys::Gfx::OpenGL::glVertexAttribP2ui
  #define glVertexAttribP2uiv ::krys::Gfx::OpenGL::glVertexAttribP2uiv
  #define glVertexAttribP3ui ::krys::Gfx::OpenGL::glVertexAttribP3ui
  #define glVertexAttribP3uiv ::krys::Gfx::OpenGL::glVertexAttribP3uiv
  #define glVertexAttribP4ui ::krys::Gfx::OpenGL::glVertexAttribP4ui
  #define glVertexAttribP4uiv ::krys::Gfx::OpenGL::glVertexAttribP4uiv
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
  #define glMinSampleShading ::krys::Gfx::OpenGL::glMinSampleShading
  #define glBlendEquationi ::krys::Gfx::OpenGL::glBlendEquationi
  #define glBlendEquationSeparatei ::krys::Gfx::OpenGL::glBlendEquationSeparatei
  #define glBlendFunci ::krys::Gfx::OpenGL::glBlendFunci
  #define glBlendFuncSeparatei ::krys::Gfx::OpenGL::glBlendFuncSeparatei
  #define glDrawArraysIndirect ::krys::Gfx::OpenGL::glDrawArraysIndirect
  #define glDrawElementsIndirect ::krys::Gfx::OpenGL::glDrawElementsIndirect
  #define glUniform1d ::krys::Gfx::OpenGL::glUniform1d
  #define glUniform2d ::krys::Gfx::OpenGL::glUniform2d
  #define glUniform3d ::krys::Gfx::OpenGL::glUniform3d
  #define glUniform4d ::krys::Gfx::OpenGL::glUniform4d
  #define glUniform1dv ::krys::Gfx::OpenGL::glUniform1dv
  #define glUniform2dv ::krys::Gfx::OpenGL::glUniform2dv
  #define glUniform3dv ::krys::Gfx::OpenGL::glUniform3dv
  #define glUniform4dv ::krys::Gfx::OpenGL::glUniform4dv
  #define glUniformMatrix2dv ::krys::Gfx::OpenGL::glUniformMatrix2dv
  #define glUniformMatrix3dv ::krys::Gfx::OpenGL::glUniformMatrix3dv
  #define glUniformMatrix4dv ::krys::Gfx::OpenGL::glUniformMatrix4dv
  #define glUniformMatrix2x3dv ::krys::Gfx::OpenGL::glUniformMatrix2x3dv
  #define glUniformMatrix2x4dv ::krys::Gfx::OpenGL::glUniformMatrix2x4dv
  #define glUniformMatrix3x2dv ::krys::Gfx::OpenGL::glUniformMatrix3x2dv
  #define glUniformMatrix3x4dv ::krys::Gfx::OpenGL::glUniformMatrix3x4dv
  #define glUniformMatrix4x2dv ::krys::Gfx::OpenGL::glUniformMatrix4x2dv
  #define glUniformMatrix4x3dv ::krys::Gfx::OpenGL::glUniformMatrix4x3dv
  #define glGetUniformdv ::krys::Gfx::OpenGL::glGetUniformdv
  #define glGetSubroutineUniformLocation ::krys::Gfx::OpenGL::glGetSubroutineUniformLocation
  #define glGetSubroutineIndex ::krys::Gfx::OpenGL::glGetSubroutineIndex
  #define glGetActiveSubroutineUniformiv ::krys::Gfx::OpenGL::glGetActiveSubroutineUniformiv
  #define glGetActiveSubroutineUniformName ::krys::Gfx::OpenGL::glGetActiveSubroutineUniformName
  #define glGetActiveSubroutineName ::krys::Gfx::OpenGL::glGetActiveSubroutineName
  #define glUniformSubroutinesuiv ::krys::Gfx::OpenGL::glUniformSubroutinesuiv
  #define glGetUniformSubroutineuiv ::krys::Gfx::OpenGL::glGetUniformSubroutineuiv
  #define glGetProgramStageiv ::krys::Gfx::OpenGL::glGetProgramStageiv
  #define glPatchParameteri ::krys::Gfx::OpenGL::glPatchParameteri
  #define glPatchParameterfv ::krys::Gfx::OpenGL::glPatchParameterfv
  #define glBindTransformFeedback ::krys::Gfx::OpenGL::glBindTransformFeedback
  #define glDeleteTransformFeedbacks ::krys::Gfx::OpenGL::glDeleteTransformFeedbacks
  #define glGenTransformFeedbacks ::krys::Gfx::OpenGL::glGenTransformFeedbacks
  #define glIsTransformFeedback ::krys::Gfx::OpenGL::glIsTransformFeedback
  #define glPauseTransformFeedback ::krys::Gfx::OpenGL::glPauseTransformFeedback
  #define glResumeTransformFeedback ::krys::Gfx::OpenGL::glResumeTransformFeedback
  #define glDrawTransformFeedback ::krys::Gfx::OpenGL::glDrawTransformFeedback
  #define glDrawTransformFeedbackStream ::krys::Gfx::OpenGL::glDrawTransformFeedbackStream
  #define glBeginQueryIndexed ::krys::Gfx::OpenGL::glBeginQueryIndexed
  #define glEndQueryIndexed ::krys::Gfx::OpenGL::glEndQueryIndexed
  #define glGetQueryIndexediv ::krys::Gfx::OpenGL::glGetQueryIndexediv
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

  KRYS_NODISCARD GLuint glCreateShaderProgramv(GLenum type, GLsizei count,
                                               const GLchar *const *strings) noexcept;

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
  #define glReleaseShaderCompiler ::krys::Gfx::OpenGL::glReleaseShaderCompiler
  #define glShaderBinary ::krys::Gfx::OpenGL::glShaderBinary
  #define glGetShaderPrecisionFormat ::krys::Gfx::OpenGL::glGetShaderPrecisionFormat
  #define glDepthRangef ::krys::Gfx::OpenGL::glDepthRangef
  #define glClearDepthf ::krys::Gfx::OpenGL::glClearDepthf
  #define glGetProgramBinary ::krys::Gfx::OpenGL::glGetProgramBinary
  #define glProgramBinary ::krys::Gfx::OpenGL::glProgramBinary
  #define glProgramParameteri ::krys::Gfx::OpenGL::glProgramParameteri
  #define glUseProgramStages ::krys::Gfx::OpenGL::glUseProgramStages
  #define glActiveShaderProgram ::krys::Gfx::OpenGL::glActiveShaderProgram
  #define glCreateShaderProgramv ::krys::Gfx::OpenGL::glCreateShaderProgramv
  #define glBindProgramPipeline ::krys::Gfx::OpenGL::glBindProgramPipeline
  #define glDeleteProgramPipelines ::krys::Gfx::OpenGL::glDeleteProgramPipelines
  #define glGenProgramPipelines ::krys::Gfx::OpenGL::glGenProgramPipelines
  #define glIsProgramPipeline ::krys::Gfx::OpenGL::glIsProgramPipeline
  #define glGetProgramPipelineiv ::krys::Gfx::OpenGL::glGetProgramPipelineiv
  #define glProgramUniform1i ::krys::Gfx::OpenGL::glProgramUniform1i
  #define glProgramUniform1iv ::krys::Gfx::OpenGL::glProgramUniform1iv
  #define glProgramUniform1f ::krys::Gfx::OpenGL::glProgramUniform1f
  #define glProgramUniform1fv ::krys::Gfx::OpenGL::glProgramUniform1fv
  #define glProgramUniform1d ::krys::Gfx::OpenGL::glProgramUniform1d
  #define glProgramUniform1dv ::krys::Gfx::OpenGL::glProgramUniform1dv
  #define glProgramUniform1ui ::krys::Gfx::OpenGL::glProgramUniform1ui
  #define glProgramUniform1uiv ::krys::Gfx::OpenGL::glProgramUniform1uiv
  #define glProgramUniform2i ::krys::Gfx::OpenGL::glProgramUniform2i
  #define glProgramUniform2iv ::krys::Gfx::OpenGL::glProgramUniform2iv
  #define glProgramUniform2f ::krys::Gfx::OpenGL::glProgramUniform2f
  #define glProgramUniform2fv ::krys::Gfx::OpenGL::glProgramUniform2fv
  #define glProgramUniform2d ::krys::Gfx::OpenGL::glProgramUniform2d
  #define glProgramUniform2dv ::krys::Gfx::OpenGL::glProgramUniform2dv
  #define glProgramUniform2ui ::krys::Gfx::OpenGL::glProgramUniform2ui
  #define glProgramUniform2uiv ::krys::Gfx::OpenGL::glProgramUniform2uiv
  #define glProgramUniform3i ::krys::Gfx::OpenGL::glProgramUniform3i
  #define glProgramUniform3iv ::krys::Gfx::OpenGL::glProgramUniform3iv
  #define glProgramUniform3f ::krys::Gfx::OpenGL::glProgramUniform3f
  #define glProgramUniform3fv ::krys::Gfx::OpenGL::glProgramUniform3fv
  #define glProgramUniform3d ::krys::Gfx::OpenGL::glProgramUniform3d
  #define glProgramUniform3dv ::krys::Gfx::OpenGL::glProgramUniform3dv
  #define glProgramUniform3ui ::krys::Gfx::OpenGL::glProgramUniform3ui
  #define glProgramUniform3uiv ::krys::Gfx::OpenGL::glProgramUniform3uiv
  #define glProgramUniform4i ::krys::Gfx::OpenGL::glProgramUniform4i
  #define glProgramUniform4iv ::krys::Gfx::OpenGL::glProgramUniform4iv
  #define glProgramUniform4f ::krys::Gfx::OpenGL::glProgramUniform4f
  #define glProgramUniform4fv ::krys::Gfx::OpenGL::glProgramUniform4fv
  #define glProgramUniform4d ::krys::Gfx::OpenGL::glProgramUniform4d
  #define glProgramUniform4dv ::krys::Gfx::OpenGL::glProgramUniform4dv
  #define glProgramUniform4ui ::krys::Gfx::OpenGL::glProgramUniform4ui
  #define glProgramUniform4uiv ::krys::Gfx::OpenGL::glProgramUniform4uiv
  #define glProgramUniformMatrix2fv ::krys::Gfx::OpenGL::glProgramUniformMatrix2fv
  #define glProgramUniformMatrix3fv ::krys::Gfx::OpenGL::glProgramUniformMatrix3fv
  #define glProgramUniformMatrix4fv ::krys::Gfx::OpenGL::glProgramUniformMatrix4fv
  #define glProgramUniformMatrix2dv ::krys::Gfx::OpenGL::glProgramUniformMatrix2dv
  #define glProgramUniformMatrix3dv ::krys::Gfx::OpenGL::glProgramUniformMatrix3dv
  #define glProgramUniformMatrix4dv ::krys::Gfx::OpenGL::glProgramUniformMatrix4dv
  #define glProgramUniformMatrix2x3fv ::krys::Gfx::OpenGL::glProgramUniformMatrix2x3fv
  #define glProgramUniformMatrix3x2fv ::krys::Gfx::OpenGL::glProgramUniformMatrix3x2fv
  #define glProgramUniformMatrix2x4fv ::krys::Gfx::OpenGL::glProgramUniformMatrix2x4fv
  #define glProgramUniformMatrix4x2fv ::krys::Gfx::OpenGL::glProgramUniformMatrix4x2fv
  #define glProgramUniformMatrix3x4fv ::krys::Gfx::OpenGL::glProgramUniformMatrix3x4fv
  #define glProgramUniformMatrix4x3fv ::krys::Gfx::OpenGL::glProgramUniformMatrix4x3fv
  #define glProgramUniformMatrix2x3dv ::krys::Gfx::OpenGL::glProgramUniformMatrix2x3dv
  #define glProgramUniformMatrix3x2dv ::krys::Gfx::OpenGL::glProgramUniformMatrix3x2dv
  #define glProgramUniformMatrix2x4dv ::krys::Gfx::OpenGL::glProgramUniformMatrix2x4dv
  #define glProgramUniformMatrix4x2dv ::krys::Gfx::OpenGL::glProgramUniformMatrix4x2dv
  #define glProgramUniformMatrix3x4dv ::krys::Gfx::OpenGL::glProgramUniformMatrix3x4dv
  #define glProgramUniformMatrix4x3dv ::krys::Gfx::OpenGL::glProgramUniformMatrix4x3dv
  #define glValidateProgramPipeline ::krys::Gfx::OpenGL::glValidateProgramPipeline
  #define glGetProgramPipelineInfoLog ::krys::Gfx::OpenGL::glGetProgramPipelineInfoLog
  #define glVertexAttribL1d ::krys::Gfx::OpenGL::glVertexAttribL1d
  #define glVertexAttribL2d ::krys::Gfx::OpenGL::glVertexAttribL2d
  #define glVertexAttribL3d ::krys::Gfx::OpenGL::glVertexAttribL3d
  #define glVertexAttribL4d ::krys::Gfx::OpenGL::glVertexAttribL4d
  #define glVertexAttribL1dv ::krys::Gfx::OpenGL::glVertexAttribL1dv
  #define glVertexAttribL2dv ::krys::Gfx::OpenGL::glVertexAttribL2dv
  #define glVertexAttribL3dv ::krys::Gfx::OpenGL::glVertexAttribL3dv
  #define glVertexAttribL4dv ::krys::Gfx::OpenGL::glVertexAttribL4dv
  #define glVertexAttribLPointer ::krys::Gfx::OpenGL::glVertexAttribLPointer
  #define glGetVertexAttribLdv ::krys::Gfx::OpenGL::glGetVertexAttribLdv
  #define glViewportArrayv ::krys::Gfx::OpenGL::glViewportArrayv
  #define glViewportIndexedf ::krys::Gfx::OpenGL::glViewportIndexedf
  #define glViewportIndexedfv ::krys::Gfx::OpenGL::glViewportIndexedfv
  #define glScissorArrayv ::krys::Gfx::OpenGL::glScissorArrayv
  #define glScissorIndexed ::krys::Gfx::OpenGL::glScissorIndexed
  #define glScissorIndexedv ::krys::Gfx::OpenGL::glScissorIndexedv
  #define glDepthRangeArrayv ::krys::Gfx::OpenGL::glDepthRangeArrayv
  #define glDepthRangeIndexed ::krys::Gfx::OpenGL::glDepthRangeIndexed
  #define glGetFloati_v ::krys::Gfx::OpenGL::glGetFloati_v
  #define glGetDoublei_v ::krys::Gfx::OpenGL::glGetDoublei_v
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
  #define glDrawArraysInstancedBaseInstance ::krys::Gfx::OpenGL::glDrawArraysInstancedBaseInstance
  #define glDrawElementsInstancedBaseInstance ::krys::Gfx::OpenGL::glDrawElementsInstancedBaseInstance
  #define glDrawElementsInstancedBaseVertexBaseInstance                                                      \
    ::krys::Gfx::OpenGL::glDrawElementsInstancedBaseVertexBaseInstance
  #define glGetInternalformativ ::krys::Gfx::OpenGL::glGetInternalformativ
  #define glGetActiveAtomicCounterBufferiv ::krys::Gfx::OpenGL::glGetActiveAtomicCounterBufferiv
  #define glBindImageTexture ::krys::Gfx::OpenGL::glBindImageTexture
  #define glMemoryBarrier ::krys::Gfx::OpenGL::glMemoryBarrier
  #define glTexStorage1D ::krys::Gfx::OpenGL::glTexStorage1D
  #define glTexStorage2D ::krys::Gfx::OpenGL::glTexStorage2D
  #define glTexStorage3D ::krys::Gfx::OpenGL::glTexStorage3D
  #define glDrawTransformFeedbackInstanced ::krys::Gfx::OpenGL::glDrawTransformFeedbackInstanced
  #define glDrawTransformFeedbackStreamInstanced ::krys::Gfx::OpenGL::glDrawTransformFeedbackStreamInstanced
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
  #define glClearBufferData ::krys::Gfx::OpenGL::glClearBufferData
  #define glClearBufferSubData ::krys::Gfx::OpenGL::glClearBufferSubData
  #define glDispatchCompute ::krys::Gfx::OpenGL::glDispatchCompute
  #define glDispatchComputeIndirect ::krys::Gfx::OpenGL::glDispatchComputeIndirect
  #define glCopyImageSubData ::krys::Gfx::OpenGL::glCopyImageSubData
  #define glFramebufferParameteri ::krys::Gfx::OpenGL::glFramebufferParameteri
  #define glGetFramebufferParameteriv ::krys::Gfx::OpenGL::glGetFramebufferParameteriv
  #define glGetInternalformati64v ::krys::Gfx::OpenGL::glGetInternalformati64v
  #define glInvalidateTexSubImage ::krys::Gfx::OpenGL::glInvalidateTexSubImage
  #define glInvalidateTexImage ::krys::Gfx::OpenGL::glInvalidateTexImage
  #define glInvalidateBufferSubData ::krys::Gfx::OpenGL::glInvalidateBufferSubData
  #define glInvalidateBufferData ::krys::Gfx::OpenGL::glInvalidateBufferData
  #define glInvalidateFramebuffer ::krys::Gfx::OpenGL::glInvalidateFramebuffer
  #define glInvalidateSubFramebuffer ::krys::Gfx::OpenGL::glInvalidateSubFramebuffer
  #define glMultiDrawArraysIndirect ::krys::Gfx::OpenGL::glMultiDrawArraysIndirect
  #define glMultiDrawElementsIndirect ::krys::Gfx::OpenGL::glMultiDrawElementsIndirect
  #define glGetProgramInterfaceiv ::krys::Gfx::OpenGL::glGetProgramInterfaceiv
  #define glGetProgramResourceIndex ::krys::Gfx::OpenGL::glGetProgramResourceIndex
  #define glGetProgramResourceName ::krys::Gfx::OpenGL::glGetProgramResourceName
  #define glGetProgramResourceiv ::krys::Gfx::OpenGL::glGetProgramResourceiv
  #define glGetProgramResourceLocation ::krys::Gfx::OpenGL::glGetProgramResourceLocation
  #define glGetProgramResourceLocationIndex ::krys::Gfx::OpenGL::glGetProgramResourceLocationIndex
  #define glShaderStorageBlockBinding ::krys::Gfx::OpenGL::glShaderStorageBlockBinding
  #define glTexBufferRange ::krys::Gfx::OpenGL::glTexBufferRange
  #define glTexStorage2DMultisample ::krys::Gfx::OpenGL::glTexStorage2DMultisample
  #define glTexStorage3DMultisample ::krys::Gfx::OpenGL::glTexStorage3DMultisample
  #define glTextureView ::krys::Gfx::OpenGL::glTextureView
  #define glBindVertexBuffer ::krys::Gfx::OpenGL::glBindVertexBuffer
  #define glVertexAttribFormat ::krys::Gfx::OpenGL::glVertexAttribFormat
  #define glVertexAttribIFormat ::krys::Gfx::OpenGL::glVertexAttribIFormat
  #define glVertexAttribLFormat ::krys::Gfx::OpenGL::glVertexAttribLFormat
  #define glVertexAttribBinding ::krys::Gfx::OpenGL::glVertexAttribBinding
  #define glVertexBindingDivisor ::krys::Gfx::OpenGL::glVertexBindingDivisor
  #define glDebugMessageControl ::krys::Gfx::OpenGL::glDebugMessageControl
  #define glDebugMessageInsert ::krys::Gfx::OpenGL::glDebugMessageInsert
  #define glDebugMessageCallback ::krys::Gfx::OpenGL::glDebugMessageCallback
  #define glGetDebugMessageLog ::krys::Gfx::OpenGL::glGetDebugMessageLog
  #define glPushDebugGroup ::krys::Gfx::OpenGL::glPushDebugGroup
  #define glPopDebugGroup ::krys::Gfx::OpenGL::glPopDebugGroup
  #define glObjectLabel ::krys::Gfx::OpenGL::glObjectLabel
  #define glGetObjectLabel ::krys::Gfx::OpenGL::glGetObjectLabel
  #define glObjectPtrLabel ::krys::Gfx::OpenGL::glObjectPtrLabel
  #define glGetObjectPtrLabel ::krys::Gfx::OpenGL::glGetObjectPtrLabel
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
  #define glBufferStorage ::krys::Gfx::OpenGL::glBufferStorage
  #define glClearTexImage ::krys::Gfx::OpenGL::glClearTexImage
  #define glClearTexSubImage ::krys::Gfx::OpenGL::glClearTexSubImage
  #define glBindBuffersBase ::krys::Gfx::OpenGL::glBindBuffersBase
  #define glBindBuffersRange ::krys::Gfx::OpenGL::glBindBuffersRange
  #define glBindTextures ::krys::Gfx::OpenGL::glBindTextures
  #define glBindSamplers ::krys::Gfx::OpenGL::glBindSamplers
  #define glBindImageTextures ::krys::Gfx::OpenGL::glBindImageTextures
  #define glBindVertexBuffers ::krys::Gfx::OpenGL::glBindVertexBuffers
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
  #define glClipControl ::krys::Gfx::OpenGL::glClipControl
  #define glCreateTransformFeedbacks ::krys::Gfx::OpenGL::glCreateTransformFeedbacks
  #define glTransformFeedbackBufferBase ::krys::Gfx::OpenGL::glTransformFeedbackBufferBase
  #define glTransformFeedbackBufferRange ::krys::Gfx::OpenGL::glTransformFeedbackBufferRange
  #define glGetTransformFeedbackiv ::krys::Gfx::OpenGL::glGetTransformFeedbackiv
  #define glGetTransformFeedbacki_v ::krys::Gfx::OpenGL::glGetTransformFeedbacki_v
  #define glGetTransformFeedbacki64_v ::krys::Gfx::OpenGL::glGetTransformFeedbacki64_v
  #define glCreateBuffers ::krys::Gfx::OpenGL::glCreateBuffers
  #define glNamedBufferStorage ::krys::Gfx::OpenGL::glNamedBufferStorage
  #define glNamedBufferData ::krys::Gfx::OpenGL::glNamedBufferData
  #define glNamedBufferSubData ::krys::Gfx::OpenGL::glNamedBufferSubData
  #define glCopyNamedBufferSubData ::krys::Gfx::OpenGL::glCopyNamedBufferSubData
  #define glClearNamedBufferData ::krys::Gfx::OpenGL::glClearNamedBufferData
  #define glClearNamedBufferSubData ::krys::Gfx::OpenGL::glClearNamedBufferSubData
  #define glMapNamedBuffer ::krys::Gfx::OpenGL::glMapNamedBuffer
  #define glMapNamedBufferRange ::krys::Gfx::OpenGL::glMapNamedBufferRange
  #define glUnmapNamedBuffer ::krys::Gfx::OpenGL::glUnmapNamedBuffer
  #define glFlushMappedNamedBufferRange ::krys::Gfx::OpenGL::glFlushMappedNamedBufferRange
  #define glGetNamedBufferParameteriv ::krys::Gfx::OpenGL::glGetNamedBufferParameteriv
  #define glGetNamedBufferParameteri64v ::krys::Gfx::OpenGL::glGetNamedBufferParameteri64v
  #define glGetNamedBufferPointerv ::krys::Gfx::OpenGL::glGetNamedBufferPointerv
  #define glGetNamedBufferSubData ::krys::Gfx::OpenGL::glGetNamedBufferSubData
  #define glCreateFramebuffers ::krys::Gfx::OpenGL::glCreateFramebuffers
  #define glNamedFramebufferRenderbuffer ::krys::Gfx::OpenGL::glNamedFramebufferRenderbuffer
  #define glNamedFramebufferParameteri ::krys::Gfx::OpenGL::glNamedFramebufferParameteri
  #define glNamedFramebufferTexture ::krys::Gfx::OpenGL::glNamedFramebufferTexture
  #define glNamedFramebufferTextureLayer ::krys::Gfx::OpenGL::glNamedFramebufferTextureLayer
  #define glNamedFramebufferDrawBuffer ::krys::Gfx::OpenGL::glNamedFramebufferDrawBuffer
  #define glNamedFramebufferDrawBuffers ::krys::Gfx::OpenGL::glNamedFramebufferDrawBuffers
  #define glNamedFramebufferReadBuffer ::krys::Gfx::OpenGL::glNamedFramebufferReadBuffer
  #define glInvalidateNamedFramebufferData ::krys::Gfx::OpenGL::glInvalidateNamedFramebufferData
  #define glInvalidateNamedFramebufferSubData ::krys::Gfx::OpenGL::glInvalidateNamedFramebufferSubData
  #define glClearNamedFramebufferiv ::krys::Gfx::OpenGL::glClearNamedFramebufferiv
  #define glClearNamedFramebufferuiv ::krys::Gfx::OpenGL::glClearNamedFramebufferuiv
  #define glClearNamedFramebufferfv ::krys::Gfx::OpenGL::glClearNamedFramebufferfv
  #define glClearNamedFramebufferfi ::krys::Gfx::OpenGL::glClearNamedFramebufferfi
  #define glBlitNamedFramebuffer ::krys::Gfx::OpenGL::glBlitNamedFramebuffer
  #define glCheckNamedFramebufferStatus ::krys::Gfx::OpenGL::glCheckNamedFramebufferStatus
  #define glGetNamedFramebufferParameteriv ::krys::Gfx::OpenGL::glGetNamedFramebufferParameteriv
  #define glGetNamedFramebufferAttachmentParameteriv                                                         \
    ::krys::Gfx::OpenGL::glGetNamedFramebufferAttachmentParameteriv
  #define glCreateRenderbuffers ::krys::Gfx::OpenGL::glCreateRenderbuffers
  #define glNamedRenderbufferStorage ::krys::Gfx::OpenGL::glNamedRenderbufferStorage
  #define glNamedRenderbufferStorageMultisample ::krys::Gfx::OpenGL::glNamedRenderbufferStorageMultisample
  #define glGetNamedRenderbufferParameteriv ::krys::Gfx::OpenGL::glGetNamedRenderbufferParameteriv
  #define glCreateTextures ::krys::Gfx::OpenGL::glCreateTextures
  #define glTextureBuffer ::krys::Gfx::OpenGL::glTextureBuffer
  #define glTextureBufferRange ::krys::Gfx::OpenGL::glTextureBufferRange
  #define glTextureStorage1D ::krys::Gfx::OpenGL::glTextureStorage1D
  #define glTextureStorage2D ::krys::Gfx::OpenGL::glTextureStorage2D
  #define glTextureStorage3D ::krys::Gfx::OpenGL::glTextureStorage3D
  #define glTextureStorage2DMultisample ::krys::Gfx::OpenGL::glTextureStorage2DMultisample
  #define glTextureStorage3DMultisample ::krys::Gfx::OpenGL::glTextureStorage3DMultisample
  #define glTextureSubImage1D ::krys::Gfx::OpenGL::glTextureSubImage1D
  #define glTextureSubImage2D ::krys::Gfx::OpenGL::glTextureSubImage2D
  #define glTextureSubImage3D ::krys::Gfx::OpenGL::glTextureSubImage3D
  #define glCompressedTextureSubImage1D ::krys::Gfx::OpenGL::glCompressedTextureSubImage1D
  #define glCompressedTextureSubImage2D ::krys::Gfx::OpenGL::glCompressedTextureSubImage2D
  #define glCompressedTextureSubImage3D ::krys::Gfx::OpenGL::glCompressedTextureSubImage3D
  #define glCopyTextureSubImage1D ::krys::Gfx::OpenGL::glCopyTextureSubImage1D
  #define glCopyTextureSubImage2D ::krys::Gfx::OpenGL::glCopyTextureSubImage2D
  #define glCopyTextureSubImage3D ::krys::Gfx::OpenGL::glCopyTextureSubImage3D
  #define glTextureParameterf ::krys::Gfx::OpenGL::glTextureParameterf
  #define glTextureParameterfv ::krys::Gfx::OpenGL::glTextureParameterfv
  #define glTextureParameteri ::krys::Gfx::OpenGL::glTextureParameteri
  #define glTextureParameterIiv ::krys::Gfx::OpenGL::glTextureParameterIiv
  #define glTextureParameterIuiv ::krys::Gfx::OpenGL::glTextureParameterIuiv
  #define glTextureParameteriv ::krys::Gfx::OpenGL::glTextureParameteriv
  #define glGenerateTextureMipmap ::krys::Gfx::OpenGL::glGenerateTextureMipmap
  #define glBindTextureUnit ::krys::Gfx::OpenGL::glBindTextureUnit
  #define glGetTextureImage ::krys::Gfx::OpenGL::glGetTextureImage
  #define glGetCompressedTextureImage ::krys::Gfx::OpenGL::glGetCompressedTextureImage
  #define glGetTextureLevelParameterfv ::krys::Gfx::OpenGL::glGetTextureLevelParameterfv
  #define glGetTextureLevelParameteriv ::krys::Gfx::OpenGL::glGetTextureLevelParameteriv
  #define glGetTextureParameterfv ::krys::Gfx::OpenGL::glGetTextureParameterfv
  #define glGetTextureParameterIiv ::krys::Gfx::OpenGL::glGetTextureParameterIiv
  #define glGetTextureParameterIuiv ::krys::Gfx::OpenGL::glGetTextureParameterIuiv
  #define glGetTextureParameteriv ::krys::Gfx::OpenGL::glGetTextureParameteriv
  #define glCreateVertexArrays ::krys::Gfx::OpenGL::glCreateVertexArrays
  #define glDisableVertexArrayAttrib ::krys::Gfx::OpenGL::glDisableVertexArrayAttrib
  #define glEnableVertexArrayAttrib ::krys::Gfx::OpenGL::glEnableVertexArrayAttrib
  #define glVertexArrayElementBuffer ::krys::Gfx::OpenGL::glVertexArrayElementBuffer
  #define glVertexArrayVertexBuffer ::krys::Gfx::OpenGL::glVertexArrayVertexBuffer
  #define glVertexArrayVertexBuffers ::krys::Gfx::OpenGL::glVertexArrayVertexBuffers
  #define glVertexArrayAttribBinding ::krys::Gfx::OpenGL::glVertexArrayAttribBinding
  #define glVertexArrayAttribFormat ::krys::Gfx::OpenGL::glVertexArrayAttribFormat
  #define glVertexArrayAttribIFormat ::krys::Gfx::OpenGL::glVertexArrayAttribIFormat
  #define glVertexArrayAttribLFormat ::krys::Gfx::OpenGL::glVertexArrayAttribLFormat
  #define glVertexArrayBindingDivisor ::krys::Gfx::OpenGL::glVertexArrayBindingDivisor
  #define glGetVertexArrayiv ::krys::Gfx::OpenGL::glGetVertexArrayiv
  #define glGetVertexArrayIndexediv ::krys::Gfx::OpenGL::glGetVertexArrayIndexediv
  #define glGetVertexArrayIndexed64iv ::krys::Gfx::OpenGL::glGetVertexArrayIndexed64iv
  #define glCreateSamplers ::krys::Gfx::OpenGL::glCreateSamplers
  #define glCreateProgramPipelines ::krys::Gfx::OpenGL::glCreateProgramPipelines
  #define glCreateQueries ::krys::Gfx::OpenGL::glCreateQueries
  #define glGetQueryBufferObjecti64v ::krys::Gfx::OpenGL::glGetQueryBufferObjecti64v
  #define glGetQueryBufferObjectiv ::krys::Gfx::OpenGL::glGetQueryBufferObjectiv
  #define glGetQueryBufferObjectui64v ::krys::Gfx::OpenGL::glGetQueryBufferObjectui64v
  #define glGetQueryBufferObjectuiv ::krys::Gfx::OpenGL::glGetQueryBufferObjectuiv
  #define glMemoryBarrierByRegion ::krys::Gfx::OpenGL::glMemoryBarrierByRegion
  #define glGetTextureSubImage ::krys::Gfx::OpenGL::glGetTextureSubImage
  #define glGetCompressedTextureSubImage ::krys::Gfx::OpenGL::glGetCompressedTextureSubImage
  #define glGetGraphicsResetStatus ::krys::Gfx::OpenGL::glGetGraphicsResetStatus
  #define glGetnCompressedTexImage ::krys::Gfx::OpenGL::glGetnCompressedTexImage
  #define glGetnTexImage ::krys::Gfx::OpenGL::glGetnTexImage
  #define glGetnUniformdv ::krys::Gfx::OpenGL::glGetnUniformdv
  #define glGetnUniformfv ::krys::Gfx::OpenGL::glGetnUniformfv
  #define glGetnUniformiv ::krys::Gfx::OpenGL::glGetnUniformiv
  #define glGetnUniformuiv ::krys::Gfx::OpenGL::glGetnUniformuiv
  #define glReadnPixels ::krys::Gfx::OpenGL::glReadnPixels
  #define glTextureBarrier ::krys::Gfx::OpenGL::glTextureBarrier
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