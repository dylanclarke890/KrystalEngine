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

#include <Krystal.Gfx/OpenGL/Official/glcorearb.h>

#pragma endregion

namespace krys::gfx::opengl
{
  /// @brief Initializes OpenGL hooks.
  KRYS_NODISCARD bool InitialiseGLHooks() noexcept;

  /// @brief Checks if a specific OpenGL extension is supported.
  KRYS_NODISCARD bool IsGLExtensionSupported(const char *extension) noexcept;
}

// OpenGL hook declarations
namespace krys::gfx::opengl
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
  #define glCullFace ::krys::gfx::opengl::glCullFace
  #define glFrontFace ::krys::gfx::opengl::glFrontFace
  #define glHint ::krys::gfx::opengl::glHint
  #define glLineWidth ::krys::gfx::opengl::glLineWidth
  #define glPointSize ::krys::gfx::opengl::glPointSize
  #define glPolygonMode ::krys::gfx::opengl::glPolygonMode
  #define glScissor ::krys::gfx::opengl::glScissor
  #define glTexParameterf ::krys::gfx::opengl::glTexParameterf
  #define glTexParameterfv ::krys::gfx::opengl::glTexParameterfv
  #define glTexParameteri ::krys::gfx::opengl::glTexParameteri
  #define glTexParameteriv ::krys::gfx::opengl::glTexParameteriv
  #define glTexImage1D ::krys::gfx::opengl::glTexImage1D
  #define glTexImage2D ::krys::gfx::opengl::glTexImage2D
  #define glDrawBuffer ::krys::gfx::opengl::glDrawBuffer
  #define glClear ::krys::gfx::opengl::glClear
  #define glClearColor ::krys::gfx::opengl::glClearColor
  #define glClearStencil ::krys::gfx::opengl::glClearStencil
  #define glClearDepth ::krys::gfx::opengl::glClearDepth
  #define glStencilMask ::krys::gfx::opengl::glStencilMask
  #define glColorMask ::krys::gfx::opengl::glColorMask
  #define glDepthMask ::krys::gfx::opengl::glDepthMask
  #define glDisable ::krys::gfx::opengl::glDisable
  #define glEnable ::krys::gfx::opengl::glEnable
  #define glFinish ::krys::gfx::opengl::glFinish
  #define glFlush ::krys::gfx::opengl::glFlush
  #define glBlendFunc ::krys::gfx::opengl::glBlendFunc
  #define glLogicOp ::krys::gfx::opengl::glLogicOp
  #define glStencilFunc ::krys::gfx::opengl::glStencilFunc
  #define glStencilOp ::krys::gfx::opengl::glStencilOp
  #define glDepthFunc ::krys::gfx::opengl::glDepthFunc
  #define glPixelStoref ::krys::gfx::opengl::glPixelStoref
  #define glPixelStorei ::krys::gfx::opengl::glPixelStorei
  #define glReadBuffer ::krys::gfx::opengl::glReadBuffer
  #define glReadPixels ::krys::gfx::opengl::glReadPixels
  #define glGetBooleanv ::krys::gfx::opengl::glGetBooleanv
  #define glGetDoublev ::krys::gfx::opengl::glGetDoublev
  #define glGetError ::krys::gfx::opengl::glGetError
  #define glGetFloatv ::krys::gfx::opengl::glGetFloatv
  #define glGetIntegerv ::krys::gfx::opengl::glGetIntegerv
  #define glGetString ::krys::gfx::opengl::glGetString
  #define glGetTexImage ::krys::gfx::opengl::glGetTexImage
  #define glGetTexParameterfv ::krys::gfx::opengl::glGetTexParameterfv
  #define glGetTexParameteriv ::krys::gfx::opengl::glGetTexParameteriv
  #define glGetTexLevelParameterfv ::krys::gfx::opengl::glGetTexLevelParameterfv
  #define glGetTexLevelParameteriv ::krys::gfx::opengl::glGetTexLevelParameteriv
  #define glIsEnabled ::krys::gfx::opengl::glIsEnabled
  #define glDepthRange ::krys::gfx::opengl::glDepthRange
  #define glViewport ::krys::gfx::opengl::glViewport
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
  #define glDrawArrays ::krys::gfx::opengl::glDrawArrays
  #define glDrawElements ::krys::gfx::opengl::glDrawElements
  #define glGetPointerv ::krys::gfx::opengl::glGetPointerv
  #define glPolygonOffset ::krys::gfx::opengl::glPolygonOffset
  #define glCopyTexImage1D ::krys::gfx::opengl::glCopyTexImage1D
  #define glCopyTexImage2D ::krys::gfx::opengl::glCopyTexImage2D
  #define glCopyTexSubImage1D ::krys::gfx::opengl::glCopyTexSubImage1D
  #define glCopyTexSubImage2D ::krys::gfx::opengl::glCopyTexSubImage2D
  #define glTexSubImage1D ::krys::gfx::opengl::glTexSubImage1D
  #define glTexSubImage2D ::krys::gfx::opengl::glTexSubImage2D
  #define glBindTexture ::krys::gfx::opengl::glBindTexture
  #define glDeleteTextures ::krys::gfx::opengl::glDeleteTextures
  #define glGenTextures ::krys::gfx::opengl::glGenTextures
  #define glIsTexture ::krys::gfx::opengl::glIsTexture
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
  #define glDrawRangeElements ::krys::gfx::opengl::glDrawRangeElements
  #define glTexImage3D ::krys::gfx::opengl::glTexImage3D
  #define glTexSubImage3D ::krys::gfx::opengl::glTexSubImage3D
  #define glCopyTexSubImage3D ::krys::gfx::opengl::glCopyTexSubImage3D
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
  #define glActiveTexture ::krys::gfx::opengl::glActiveTexture
  #define glSampleCoverage ::krys::gfx::opengl::glSampleCoverage
  #define glCompressedTexImage3D ::krys::gfx::opengl::glCompressedTexImage3D
  #define glCompressedTexImage2D ::krys::gfx::opengl::glCompressedTexImage2D
  #define glCompressedTexImage1D ::krys::gfx::opengl::glCompressedTexImage1D
  #define glCompressedTexSubImage3D ::krys::gfx::opengl::glCompressedTexSubImage3D
  #define glCompressedTexSubImage2D ::krys::gfx::opengl::glCompressedTexSubImage2D
  #define glCompressedTexSubImage1D ::krys::gfx::opengl::glCompressedTexSubImage1D
  #define glGetCompressedTexImage ::krys::gfx::opengl::glGetCompressedTexImage
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
  #define glBlendFuncSeparate ::krys::gfx::opengl::glBlendFuncSeparate
  #define glMultiDrawArrays ::krys::gfx::opengl::glMultiDrawArrays
  #define glMultiDrawElements ::krys::gfx::opengl::glMultiDrawElements
  #define glPointParameterf ::krys::gfx::opengl::glPointParameterf
  #define glPointParameterfv ::krys::gfx::opengl::glPointParameterfv
  #define glPointParameteri ::krys::gfx::opengl::glPointParameteri
  #define glPointParameteriv ::krys::gfx::opengl::glPointParameteriv
  #define glBlendColor ::krys::gfx::opengl::glBlendColor
  #define glBlendEquation ::krys::gfx::opengl::glBlendEquation
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
  #define glGenQueries ::krys::gfx::opengl::glGenQueries
  #define glDeleteQueries ::krys::gfx::opengl::glDeleteQueries
  #define glIsQuery ::krys::gfx::opengl::glIsQuery
  #define glBeginQuery ::krys::gfx::opengl::glBeginQuery
  #define glEndQuery ::krys::gfx::opengl::glEndQuery
  #define glGetQueryiv ::krys::gfx::opengl::glGetQueryiv
  #define glGetQueryObjectiv ::krys::gfx::opengl::glGetQueryObjectiv
  #define glGetQueryObjectuiv ::krys::gfx::opengl::glGetQueryObjectuiv
  #define glBindBuffer ::krys::gfx::opengl::glBindBuffer
  #define glDeleteBuffers ::krys::gfx::opengl::glDeleteBuffers
  #define glGenBuffers ::krys::gfx::opengl::glGenBuffers
  #define glIsBuffer ::krys::gfx::opengl::glIsBuffer
  #define glBufferData ::krys::gfx::opengl::glBufferData
  #define glBufferSubData ::krys::gfx::opengl::glBufferSubData
  #define glGetBufferSubData ::krys::gfx::opengl::glGetBufferSubData
  #define glMapBuffer ::krys::gfx::opengl::glMapBuffer
  #define glUnmapBuffer ::krys::gfx::opengl::glUnmapBuffer
  #define glGetBufferParameteriv ::krys::gfx::opengl::glGetBufferParameteriv
  #define glGetBufferPointerv ::krys::gfx::opengl::glGetBufferPointerv
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
  #define glBlendEquationSeparate ::krys::gfx::opengl::glBlendEquationSeparate
  #define glDrawBuffers ::krys::gfx::opengl::glDrawBuffers
  #define glStencilOpSeparate ::krys::gfx::opengl::glStencilOpSeparate
  #define glStencilFuncSeparate ::krys::gfx::opengl::glStencilFuncSeparate
  #define glStencilMaskSeparate ::krys::gfx::opengl::glStencilMaskSeparate
  #define glAttachShader ::krys::gfx::opengl::glAttachShader
  #define glBindAttribLocation ::krys::gfx::opengl::glBindAttribLocation
  #define glCompileShader ::krys::gfx::opengl::glCompileShader
  #define glCreateProgram ::krys::gfx::opengl::glCreateProgram
  #define glCreateShader ::krys::gfx::opengl::glCreateShader
  #define glDeleteProgram ::krys::gfx::opengl::glDeleteProgram
  #define glDeleteShader ::krys::gfx::opengl::glDeleteShader
  #define glDetachShader ::krys::gfx::opengl::glDetachShader
  #define glDisableVertexAttribArray ::krys::gfx::opengl::glDisableVertexAttribArray
  #define glEnableVertexAttribArray ::krys::gfx::opengl::glEnableVertexAttribArray
  #define glGetActiveAttrib ::krys::gfx::opengl::glGetActiveAttrib
  #define glGetActiveUniform ::krys::gfx::opengl::glGetActiveUniform
  #define glGetAttachedShaders ::krys::gfx::opengl::glGetAttachedShaders
  #define glGetAttribLocation ::krys::gfx::opengl::glGetAttribLocation
  #define glGetProgramiv ::krys::gfx::opengl::glGetProgramiv
  #define glGetProgramInfoLog ::krys::gfx::opengl::glGetProgramInfoLog
  #define glGetShaderiv ::krys::gfx::opengl::glGetShaderiv
  #define glGetShaderInfoLog ::krys::gfx::opengl::glGetShaderInfoLog
  #define glGetShaderSource ::krys::gfx::opengl::glGetShaderSource
  #define glGetUniformLocation ::krys::gfx::opengl::glGetUniformLocation
  #define glGetUniformfv ::krys::gfx::opengl::glGetUniformfv
  #define glGetUniformiv ::krys::gfx::opengl::glGetUniformiv
  #define glGetVertexAttribdv ::krys::gfx::opengl::glGetVertexAttribdv
  #define glGetVertexAttribfv ::krys::gfx::opengl::glGetVertexAttribfv
  #define glGetVertexAttribiv ::krys::gfx::opengl::glGetVertexAttribiv
  #define glGetVertexAttribPointerv ::krys::gfx::opengl::glGetVertexAttribPointerv
  #define glIsProgram ::krys::gfx::opengl::glIsProgram
  #define glIsShader ::krys::gfx::opengl::glIsShader
  #define glLinkProgram ::krys::gfx::opengl::glLinkProgram
  #define glShaderSource ::krys::gfx::opengl::glShaderSource
  #define glUseProgram ::krys::gfx::opengl::glUseProgram
  #define glUniform1f ::krys::gfx::opengl::glUniform1f
  #define glUniform2f ::krys::gfx::opengl::glUniform2f
  #define glUniform3f ::krys::gfx::opengl::glUniform3f
  #define glUniform4f ::krys::gfx::opengl::glUniform4f
  #define glUniform1i ::krys::gfx::opengl::glUniform1i
  #define glUniform2i ::krys::gfx::opengl::glUniform2i
  #define glUniform3i ::krys::gfx::opengl::glUniform3i
  #define glUniform4i ::krys::gfx::opengl::glUniform4i
  #define glUniform1fv ::krys::gfx::opengl::glUniform1fv
  #define glUniform2fv ::krys::gfx::opengl::glUniform2fv
  #define glUniform3fv ::krys::gfx::opengl::glUniform3fv
  #define glUniform4fv ::krys::gfx::opengl::glUniform4fv
  #define glUniform1iv ::krys::gfx::opengl::glUniform1iv
  #define glUniform2iv ::krys::gfx::opengl::glUniform2iv
  #define glUniform3iv ::krys::gfx::opengl::glUniform3iv
  #define glUniform4iv ::krys::gfx::opengl::glUniform4iv
  #define glUniformMatrix2fv ::krys::gfx::opengl::glUniformMatrix2fv
  #define glUniformMatrix3fv ::krys::gfx::opengl::glUniformMatrix3fv
  #define glUniformMatrix4fv ::krys::gfx::opengl::glUniformMatrix4fv
  #define glValidateProgram ::krys::gfx::opengl::glValidateProgram
  #define glVertexAttrib1d ::krys::gfx::opengl::glVertexAttrib1d
  #define glVertexAttrib1dv ::krys::gfx::opengl::glVertexAttrib1dv
  #define glVertexAttrib1f ::krys::gfx::opengl::glVertexAttrib1f
  #define glVertexAttrib1fv ::krys::gfx::opengl::glVertexAttrib1fv
  #define glVertexAttrib1s ::krys::gfx::opengl::glVertexAttrib1s
  #define glVertexAttrib1sv ::krys::gfx::opengl::glVertexAttrib1sv
  #define glVertexAttrib2d ::krys::gfx::opengl::glVertexAttrib2d
  #define glVertexAttrib2dv ::krys::gfx::opengl::glVertexAttrib2dv
  #define glVertexAttrib2f ::krys::gfx::opengl::glVertexAttrib2f
  #define glVertexAttrib2fv ::krys::gfx::opengl::glVertexAttrib2fv
  #define glVertexAttrib2s ::krys::gfx::opengl::glVertexAttrib2s
  #define glVertexAttrib2sv ::krys::gfx::opengl::glVertexAttrib2sv
  #define glVertexAttrib3d ::krys::gfx::opengl::glVertexAttrib3d
  #define glVertexAttrib3dv ::krys::gfx::opengl::glVertexAttrib3dv
  #define glVertexAttrib3f ::krys::gfx::opengl::glVertexAttrib3f
  #define glVertexAttrib3fv ::krys::gfx::opengl::glVertexAttrib3fv
  #define glVertexAttrib3s ::krys::gfx::opengl::glVertexAttrib3s
  #define glVertexAttrib3sv ::krys::gfx::opengl::glVertexAttrib3sv
  #define glVertexAttrib4Nbv ::krys::gfx::opengl::glVertexAttrib4Nbv
  #define glVertexAttrib4Niv ::krys::gfx::opengl::glVertexAttrib4Niv
  #define glVertexAttrib4Nsv ::krys::gfx::opengl::glVertexAttrib4Nsv
  #define glVertexAttrib4Nub ::krys::gfx::opengl::glVertexAttrib4Nub
  #define glVertexAttrib4Nubv ::krys::gfx::opengl::glVertexAttrib4Nubv
  #define glVertexAttrib4Nuiv ::krys::gfx::opengl::glVertexAttrib4Nuiv
  #define glVertexAttrib4Nusv ::krys::gfx::opengl::glVertexAttrib4Nusv
  #define glVertexAttrib4bv ::krys::gfx::opengl::glVertexAttrib4bv
  #define glVertexAttrib4d ::krys::gfx::opengl::glVertexAttrib4d
  #define glVertexAttrib4dv ::krys::gfx::opengl::glVertexAttrib4dv
  #define glVertexAttrib4f ::krys::gfx::opengl::glVertexAttrib4f
  #define glVertexAttrib4fv ::krys::gfx::opengl::glVertexAttrib4fv
  #define glVertexAttrib4iv ::krys::gfx::opengl::glVertexAttrib4iv
  #define glVertexAttrib4s ::krys::gfx::opengl::glVertexAttrib4s
  #define glVertexAttrib4sv ::krys::gfx::opengl::glVertexAttrib4sv
  #define glVertexAttrib4ubv ::krys::gfx::opengl::glVertexAttrib4ubv
  #define glVertexAttrib4uiv ::krys::gfx::opengl::glVertexAttrib4uiv
  #define glVertexAttrib4usv ::krys::gfx::opengl::glVertexAttrib4usv
  #define glVertexAttribPointer ::krys::gfx::opengl::glVertexAttribPointer
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
  #define glUniformMatrix2x3fv ::krys::gfx::opengl::glUniformMatrix2x3fv
  #define glUniformMatrix3x2fv ::krys::gfx::opengl::glUniformMatrix3x2fv
  #define glUniformMatrix2x4fv ::krys::gfx::opengl::glUniformMatrix2x4fv
  #define glUniformMatrix4x2fv ::krys::gfx::opengl::glUniformMatrix4x2fv
  #define glUniformMatrix3x4fv ::krys::gfx::opengl::glUniformMatrix3x4fv
  #define glUniformMatrix4x3fv ::krys::gfx::opengl::glUniformMatrix4x3fv
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
  #define glColorMaski ::krys::gfx::opengl::glColorMaski
  #define glGetBooleani_v ::krys::gfx::opengl::glGetBooleani_v
  #define glGetIntegeri_v ::krys::gfx::opengl::glGetIntegeri_v
  #define glEnablei ::krys::gfx::opengl::glEnablei
  #define glDisablei ::krys::gfx::opengl::glDisablei
  #define glIsEnabledi ::krys::gfx::opengl::glIsEnabledi
  #define glBeginTransformFeedback ::krys::gfx::opengl::glBeginTransformFeedback
  #define glEndTransformFeedback ::krys::gfx::opengl::glEndTransformFeedback
  #define glBindBufferRange ::krys::gfx::opengl::glBindBufferRange
  #define glBindBufferBase ::krys::gfx::opengl::glBindBufferBase
  #define glTransformFeedbackVaryings ::krys::gfx::opengl::glTransformFeedbackVaryings
  #define glGetTransformFeedbackVarying ::krys::gfx::opengl::glGetTransformFeedbackVarying
  #define glClampColor ::krys::gfx::opengl::glClampColor
  #define glBeginConditionalRender ::krys::gfx::opengl::glBeginConditionalRender
  #define glEndConditionalRender ::krys::gfx::opengl::glEndConditionalRender
  #define glVertexAttribIPointer ::krys::gfx::opengl::glVertexAttribIPointer
  #define glGetVertexAttribIiv ::krys::gfx::opengl::glGetVertexAttribIiv
  #define glGetVertexAttribIuiv ::krys::gfx::opengl::glGetVertexAttribIuiv
  #define glVertexAttribI1i ::krys::gfx::opengl::glVertexAttribI1i
  #define glVertexAttribI2i ::krys::gfx::opengl::glVertexAttribI2i
  #define glVertexAttribI3i ::krys::gfx::opengl::glVertexAttribI3i
  #define glVertexAttribI4i ::krys::gfx::opengl::glVertexAttribI4i
  #define glVertexAttribI1ui ::krys::gfx::opengl::glVertexAttribI1ui
  #define glVertexAttribI2ui ::krys::gfx::opengl::glVertexAttribI2ui
  #define glVertexAttribI3ui ::krys::gfx::opengl::glVertexAttribI3ui
  #define glVertexAttribI4ui ::krys::gfx::opengl::glVertexAttribI4ui
  #define glVertexAttribI1iv ::krys::gfx::opengl::glVertexAttribI1iv
  #define glVertexAttribI2iv ::krys::gfx::opengl::glVertexAttribI2iv
  #define glVertexAttribI3iv ::krys::gfx::opengl::glVertexAttribI3iv
  #define glVertexAttribI4iv ::krys::gfx::opengl::glVertexAttribI4iv
  #define glVertexAttribI1uiv ::krys::gfx::opengl::glVertexAttribI1uiv
  #define glVertexAttribI2uiv ::krys::gfx::opengl::glVertexAttribI2uiv
  #define glVertexAttribI3uiv ::krys::gfx::opengl::glVertexAttribI3uiv
  #define glVertexAttribI4uiv ::krys::gfx::opengl::glVertexAttribI4uiv
  #define glVertexAttribI4bv ::krys::gfx::opengl::glVertexAttribI4bv
  #define glVertexAttribI4sv ::krys::gfx::opengl::glVertexAttribI4sv
  #define glVertexAttribI4ubv ::krys::gfx::opengl::glVertexAttribI4ubv
  #define glVertexAttribI4usv ::krys::gfx::opengl::glVertexAttribI4usv
  #define glGetUniformuiv ::krys::gfx::opengl::glGetUniformuiv
  #define glBindFragDataLocation ::krys::gfx::opengl::glBindFragDataLocation
  #define glGetFragDataLocation ::krys::gfx::opengl::glGetFragDataLocation
  #define glUniform1ui ::krys::gfx::opengl::glUniform1ui
  #define glUniform2ui ::krys::gfx::opengl::glUniform2ui
  #define glUniform3ui ::krys::gfx::opengl::glUniform3ui
  #define glUniform4ui ::krys::gfx::opengl::glUniform4ui
  #define glUniform1uiv ::krys::gfx::opengl::glUniform1uiv
  #define glUniform2uiv ::krys::gfx::opengl::glUniform2uiv
  #define glUniform3uiv ::krys::gfx::opengl::glUniform3uiv
  #define glUniform4uiv ::krys::gfx::opengl::glUniform4uiv
  #define glTexParameterIiv ::krys::gfx::opengl::glTexParameterIiv
  #define glTexParameterIuiv ::krys::gfx::opengl::glTexParameterIuiv
  #define glGetTexParameterIiv ::krys::gfx::opengl::glGetTexParameterIiv
  #define glGetTexParameterIuiv ::krys::gfx::opengl::glGetTexParameterIuiv
  #define glClearBufferiv ::krys::gfx::opengl::glClearBufferiv
  #define glClearBufferuiv ::krys::gfx::opengl::glClearBufferuiv
  #define glClearBufferfv ::krys::gfx::opengl::glClearBufferfv
  #define glClearBufferfi ::krys::gfx::opengl::glClearBufferfi
  #define glGetStringi ::krys::gfx::opengl::glGetStringi
  #define glIsRenderbuffer ::krys::gfx::opengl::glIsRenderbuffer
  #define glBindRenderbuffer ::krys::gfx::opengl::glBindRenderbuffer
  #define glDeleteRenderbuffers ::krys::gfx::opengl::glDeleteRenderbuffers
  #define glGenRenderbuffers ::krys::gfx::opengl::glGenRenderbuffers
  #define glRenderbufferStorage ::krys::gfx::opengl::glRenderbufferStorage
  #define glGetRenderbufferParameteriv ::krys::gfx::opengl::glGetRenderbufferParameteriv
  #define glIsFramebuffer ::krys::gfx::opengl::glIsFramebuffer
  #define glBindFramebuffer ::krys::gfx::opengl::glBindFramebuffer
  #define glDeleteFramebuffers ::krys::gfx::opengl::glDeleteFramebuffers
  #define glGenFramebuffers ::krys::gfx::opengl::glGenFramebuffers
  #define glCheckFramebufferStatus ::krys::gfx::opengl::glCheckFramebufferStatus
  #define glFramebufferTexture1D ::krys::gfx::opengl::glFramebufferTexture1D
  #define glFramebufferTexture2D ::krys::gfx::opengl::glFramebufferTexture2D
  #define glFramebufferTexture3D ::krys::gfx::opengl::glFramebufferTexture3D
  #define glFramebufferRenderbuffer ::krys::gfx::opengl::glFramebufferRenderbuffer
  #define glGetFramebufferAttachmentParameteriv ::krys::gfx::opengl::glGetFramebufferAttachmentParameteriv
  #define glGenerateMipmap ::krys::gfx::opengl::glGenerateMipmap
  #define glBlitFramebuffer ::krys::gfx::opengl::glBlitFramebuffer
  #define glRenderbufferStorageMultisample ::krys::gfx::opengl::glRenderbufferStorageMultisample
  #define glFramebufferTextureLayer ::krys::gfx::opengl::glFramebufferTextureLayer
  #define glMapBufferRange ::krys::gfx::opengl::glMapBufferRange
  #define glFlushMappedBufferRange ::krys::gfx::opengl::glFlushMappedBufferRange
  #define glBindVertexArray ::krys::gfx::opengl::glBindVertexArray
  #define glDeleteVertexArrays ::krys::gfx::opengl::glDeleteVertexArrays
  #define glGenVertexArrays ::krys::gfx::opengl::glGenVertexArrays
  #define glIsVertexArray ::krys::gfx::opengl::glIsVertexArray
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
  #define glDrawArraysInstanced ::krys::gfx::opengl::glDrawArraysInstanced
  #define glDrawElementsInstanced ::krys::gfx::opengl::glDrawElementsInstanced
  #define glTexBuffer ::krys::gfx::opengl::glTexBuffer
  #define glPrimitiveRestartIndex ::krys::gfx::opengl::glPrimitiveRestartIndex
  #define glCopyBufferSubData ::krys::gfx::opengl::glCopyBufferSubData
  #define glGetUniformIndices ::krys::gfx::opengl::glGetUniformIndices
  #define glGetActiveUniformsiv ::krys::gfx::opengl::glGetActiveUniformsiv
  #define glGetActiveUniformName ::krys::gfx::opengl::glGetActiveUniformName
  #define glGetUniformBlockIndex ::krys::gfx::opengl::glGetUniformBlockIndex
  #define glGetActiveUniformBlockiv ::krys::gfx::opengl::glGetActiveUniformBlockiv
  #define glGetActiveUniformBlockName ::krys::gfx::opengl::glGetActiveUniformBlockName
  #define glUniformBlockBinding ::krys::gfx::opengl::glUniformBlockBinding
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
  #define glDrawElementsBaseVertex ::krys::gfx::opengl::glDrawElementsBaseVertex
  #define glDrawRangeElementsBaseVertex ::krys::gfx::opengl::glDrawRangeElementsBaseVertex
  #define glDrawElementsInstancedBaseVertex ::krys::gfx::opengl::glDrawElementsInstancedBaseVertex
  #define glMultiDrawElementsBaseVertex ::krys::gfx::opengl::glMultiDrawElementsBaseVertex
  #define glProvokingVertex ::krys::gfx::opengl::glProvokingVertex
  #define glFenceSync ::krys::gfx::opengl::glFenceSync
  #define glIsSync ::krys::gfx::opengl::glIsSync
  #define glDeleteSync ::krys::gfx::opengl::glDeleteSync
  #define glClientWaitSync ::krys::gfx::opengl::glClientWaitSync
  #define glWaitSync ::krys::gfx::opengl::glWaitSync
  #define glGetInteger64v ::krys::gfx::opengl::glGetInteger64v
  #define glGetSynciv ::krys::gfx::opengl::glGetSynciv
  #define glGetInteger64i_v ::krys::gfx::opengl::glGetInteger64i_v
  #define glGetBufferParameteri64v ::krys::gfx::opengl::glGetBufferParameteri64v
  #define glFramebufferTexture ::krys::gfx::opengl::glFramebufferTexture
  #define glTexImage2DMultisample ::krys::gfx::opengl::glTexImage2DMultisample
  #define glTexImage3DMultisample ::krys::gfx::opengl::glTexImage3DMultisample
  #define glGetMultisamplefv ::krys::gfx::opengl::glGetMultisamplefv
  #define glSampleMaski ::krys::gfx::opengl::glSampleMaski
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
  #define glBindFragDataLocationIndexed ::krys::gfx::opengl::glBindFragDataLocationIndexed
  #define glGetFragDataIndex ::krys::gfx::opengl::glGetFragDataIndex
  #define glGenSamplers ::krys::gfx::opengl::glGenSamplers
  #define glDeleteSamplers ::krys::gfx::opengl::glDeleteSamplers
  #define glIsSampler ::krys::gfx::opengl::glIsSampler
  #define glBindSampler ::krys::gfx::opengl::glBindSampler
  #define glSamplerParameteri ::krys::gfx::opengl::glSamplerParameteri
  #define glSamplerParameteriv ::krys::gfx::opengl::glSamplerParameteriv
  #define glSamplerParameterf ::krys::gfx::opengl::glSamplerParameterf
  #define glSamplerParameterfv ::krys::gfx::opengl::glSamplerParameterfv
  #define glSamplerParameterIiv ::krys::gfx::opengl::glSamplerParameterIiv
  #define glSamplerParameterIuiv ::krys::gfx::opengl::glSamplerParameterIuiv
  #define glGetSamplerParameteriv ::krys::gfx::opengl::glGetSamplerParameteriv
  #define glGetSamplerParameterIiv ::krys::gfx::opengl::glGetSamplerParameterIiv
  #define glGetSamplerParameterfv ::krys::gfx::opengl::glGetSamplerParameterfv
  #define glGetSamplerParameterIuiv ::krys::gfx::opengl::glGetSamplerParameterIuiv
  #define glQueryCounter ::krys::gfx::opengl::glQueryCounter
  #define glGetQueryObjecti64v ::krys::gfx::opengl::glGetQueryObjecti64v
  #define glGetQueryObjectui64v ::krys::gfx::opengl::glGetQueryObjectui64v
  #define glVertexAttribDivisor ::krys::gfx::opengl::glVertexAttribDivisor
  #define glVertexAttribP1ui ::krys::gfx::opengl::glVertexAttribP1ui
  #define glVertexAttribP1uiv ::krys::gfx::opengl::glVertexAttribP1uiv
  #define glVertexAttribP2ui ::krys::gfx::opengl::glVertexAttribP2ui
  #define glVertexAttribP2uiv ::krys::gfx::opengl::glVertexAttribP2uiv
  #define glVertexAttribP3ui ::krys::gfx::opengl::glVertexAttribP3ui
  #define glVertexAttribP3uiv ::krys::gfx::opengl::glVertexAttribP3uiv
  #define glVertexAttribP4ui ::krys::gfx::opengl::glVertexAttribP4ui
  #define glVertexAttribP4uiv ::krys::gfx::opengl::glVertexAttribP4uiv
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
  #define glMinSampleShading ::krys::gfx::opengl::glMinSampleShading
  #define glBlendEquationi ::krys::gfx::opengl::glBlendEquationi
  #define glBlendEquationSeparatei ::krys::gfx::opengl::glBlendEquationSeparatei
  #define glBlendFunci ::krys::gfx::opengl::glBlendFunci
  #define glBlendFuncSeparatei ::krys::gfx::opengl::glBlendFuncSeparatei
  #define glDrawArraysIndirect ::krys::gfx::opengl::glDrawArraysIndirect
  #define glDrawElementsIndirect ::krys::gfx::opengl::glDrawElementsIndirect
  #define glUniform1d ::krys::gfx::opengl::glUniform1d
  #define glUniform2d ::krys::gfx::opengl::glUniform2d
  #define glUniform3d ::krys::gfx::opengl::glUniform3d
  #define glUniform4d ::krys::gfx::opengl::glUniform4d
  #define glUniform1dv ::krys::gfx::opengl::glUniform1dv
  #define glUniform2dv ::krys::gfx::opengl::glUniform2dv
  #define glUniform3dv ::krys::gfx::opengl::glUniform3dv
  #define glUniform4dv ::krys::gfx::opengl::glUniform4dv
  #define glUniformMatrix2dv ::krys::gfx::opengl::glUniformMatrix2dv
  #define glUniformMatrix3dv ::krys::gfx::opengl::glUniformMatrix3dv
  #define glUniformMatrix4dv ::krys::gfx::opengl::glUniformMatrix4dv
  #define glUniformMatrix2x3dv ::krys::gfx::opengl::glUniformMatrix2x3dv
  #define glUniformMatrix2x4dv ::krys::gfx::opengl::glUniformMatrix2x4dv
  #define glUniformMatrix3x2dv ::krys::gfx::opengl::glUniformMatrix3x2dv
  #define glUniformMatrix3x4dv ::krys::gfx::opengl::glUniformMatrix3x4dv
  #define glUniformMatrix4x2dv ::krys::gfx::opengl::glUniformMatrix4x2dv
  #define glUniformMatrix4x3dv ::krys::gfx::opengl::glUniformMatrix4x3dv
  #define glGetUniformdv ::krys::gfx::opengl::glGetUniformdv
  #define glGetSubroutineUniformLocation ::krys::gfx::opengl::glGetSubroutineUniformLocation
  #define glGetSubroutineIndex ::krys::gfx::opengl::glGetSubroutineIndex
  #define glGetActiveSubroutineUniformiv ::krys::gfx::opengl::glGetActiveSubroutineUniformiv
  #define glGetActiveSubroutineUniformName ::krys::gfx::opengl::glGetActiveSubroutineUniformName
  #define glGetActiveSubroutineName ::krys::gfx::opengl::glGetActiveSubroutineName
  #define glUniformSubroutinesuiv ::krys::gfx::opengl::glUniformSubroutinesuiv
  #define glGetUniformSubroutineuiv ::krys::gfx::opengl::glGetUniformSubroutineuiv
  #define glGetProgramStageiv ::krys::gfx::opengl::glGetProgramStageiv
  #define glPatchParameteri ::krys::gfx::opengl::glPatchParameteri
  #define glPatchParameterfv ::krys::gfx::opengl::glPatchParameterfv
  #define glBindTransformFeedback ::krys::gfx::opengl::glBindTransformFeedback
  #define glDeleteTransformFeedbacks ::krys::gfx::opengl::glDeleteTransformFeedbacks
  #define glGenTransformFeedbacks ::krys::gfx::opengl::glGenTransformFeedbacks
  #define glIsTransformFeedback ::krys::gfx::opengl::glIsTransformFeedback
  #define glPauseTransformFeedback ::krys::gfx::opengl::glPauseTransformFeedback
  #define glResumeTransformFeedback ::krys::gfx::opengl::glResumeTransformFeedback
  #define glDrawTransformFeedback ::krys::gfx::opengl::glDrawTransformFeedback
  #define glDrawTransformFeedbackStream ::krys::gfx::opengl::glDrawTransformFeedbackStream
  #define glBeginQueryIndexed ::krys::gfx::opengl::glBeginQueryIndexed
  #define glEndQueryIndexed ::krys::gfx::opengl::glEndQueryIndexed
  #define glGetQueryIndexediv ::krys::gfx::opengl::glGetQueryIndexediv
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
  #define glReleaseShaderCompiler ::krys::gfx::opengl::glReleaseShaderCompiler
  #define glShaderBinary ::krys::gfx::opengl::glShaderBinary
  #define glGetShaderPrecisionFormat ::krys::gfx::opengl::glGetShaderPrecisionFormat
  #define glDepthRangef ::krys::gfx::opengl::glDepthRangef
  #define glClearDepthf ::krys::gfx::opengl::glClearDepthf
  #define glGetProgramBinary ::krys::gfx::opengl::glGetProgramBinary
  #define glProgramBinary ::krys::gfx::opengl::glProgramBinary
  #define glProgramParameteri ::krys::gfx::opengl::glProgramParameteri
  #define glUseProgramStages ::krys::gfx::opengl::glUseProgramStages
  #define glActiveShaderProgram ::krys::gfx::opengl::glActiveShaderProgram
  #define glCreateShaderProgramv ::krys::gfx::opengl::glCreateShaderProgramv
  #define glBindProgramPipeline ::krys::gfx::opengl::glBindProgramPipeline
  #define glDeleteProgramPipelines ::krys::gfx::opengl::glDeleteProgramPipelines
  #define glGenProgramPipelines ::krys::gfx::opengl::glGenProgramPipelines
  #define glIsProgramPipeline ::krys::gfx::opengl::glIsProgramPipeline
  #define glGetProgramPipelineiv ::krys::gfx::opengl::glGetProgramPipelineiv
  #define glProgramUniform1i ::krys::gfx::opengl::glProgramUniform1i
  #define glProgramUniform1iv ::krys::gfx::opengl::glProgramUniform1iv
  #define glProgramUniform1f ::krys::gfx::opengl::glProgramUniform1f
  #define glProgramUniform1fv ::krys::gfx::opengl::glProgramUniform1fv
  #define glProgramUniform1d ::krys::gfx::opengl::glProgramUniform1d
  #define glProgramUniform1dv ::krys::gfx::opengl::glProgramUniform1dv
  #define glProgramUniform1ui ::krys::gfx::opengl::glProgramUniform1ui
  #define glProgramUniform1uiv ::krys::gfx::opengl::glProgramUniform1uiv
  #define glProgramUniform2i ::krys::gfx::opengl::glProgramUniform2i
  #define glProgramUniform2iv ::krys::gfx::opengl::glProgramUniform2iv
  #define glProgramUniform2f ::krys::gfx::opengl::glProgramUniform2f
  #define glProgramUniform2fv ::krys::gfx::opengl::glProgramUniform2fv
  #define glProgramUniform2d ::krys::gfx::opengl::glProgramUniform2d
  #define glProgramUniform2dv ::krys::gfx::opengl::glProgramUniform2dv
  #define glProgramUniform2ui ::krys::gfx::opengl::glProgramUniform2ui
  #define glProgramUniform2uiv ::krys::gfx::opengl::glProgramUniform2uiv
  #define glProgramUniform3i ::krys::gfx::opengl::glProgramUniform3i
  #define glProgramUniform3iv ::krys::gfx::opengl::glProgramUniform3iv
  #define glProgramUniform3f ::krys::gfx::opengl::glProgramUniform3f
  #define glProgramUniform3fv ::krys::gfx::opengl::glProgramUniform3fv
  #define glProgramUniform3d ::krys::gfx::opengl::glProgramUniform3d
  #define glProgramUniform3dv ::krys::gfx::opengl::glProgramUniform3dv
  #define glProgramUniform3ui ::krys::gfx::opengl::glProgramUniform3ui
  #define glProgramUniform3uiv ::krys::gfx::opengl::glProgramUniform3uiv
  #define glProgramUniform4i ::krys::gfx::opengl::glProgramUniform4i
  #define glProgramUniform4iv ::krys::gfx::opengl::glProgramUniform4iv
  #define glProgramUniform4f ::krys::gfx::opengl::glProgramUniform4f
  #define glProgramUniform4fv ::krys::gfx::opengl::glProgramUniform4fv
  #define glProgramUniform4d ::krys::gfx::opengl::glProgramUniform4d
  #define glProgramUniform4dv ::krys::gfx::opengl::glProgramUniform4dv
  #define glProgramUniform4ui ::krys::gfx::opengl::glProgramUniform4ui
  #define glProgramUniform4uiv ::krys::gfx::opengl::glProgramUniform4uiv
  #define glProgramUniformMatrix2fv ::krys::gfx::opengl::glProgramUniformMatrix2fv
  #define glProgramUniformMatrix3fv ::krys::gfx::opengl::glProgramUniformMatrix3fv
  #define glProgramUniformMatrix4fv ::krys::gfx::opengl::glProgramUniformMatrix4fv
  #define glProgramUniformMatrix2dv ::krys::gfx::opengl::glProgramUniformMatrix2dv
  #define glProgramUniformMatrix3dv ::krys::gfx::opengl::glProgramUniformMatrix3dv
  #define glProgramUniformMatrix4dv ::krys::gfx::opengl::glProgramUniformMatrix4dv
  #define glProgramUniformMatrix2x3fv ::krys::gfx::opengl::glProgramUniformMatrix2x3fv
  #define glProgramUniformMatrix3x2fv ::krys::gfx::opengl::glProgramUniformMatrix3x2fv
  #define glProgramUniformMatrix2x4fv ::krys::gfx::opengl::glProgramUniformMatrix2x4fv
  #define glProgramUniformMatrix4x2fv ::krys::gfx::opengl::glProgramUniformMatrix4x2fv
  #define glProgramUniformMatrix3x4fv ::krys::gfx::opengl::glProgramUniformMatrix3x4fv
  #define glProgramUniformMatrix4x3fv ::krys::gfx::opengl::glProgramUniformMatrix4x3fv
  #define glProgramUniformMatrix2x3dv ::krys::gfx::opengl::glProgramUniformMatrix2x3dv
  #define glProgramUniformMatrix3x2dv ::krys::gfx::opengl::glProgramUniformMatrix3x2dv
  #define glProgramUniformMatrix2x4dv ::krys::gfx::opengl::glProgramUniformMatrix2x4dv
  #define glProgramUniformMatrix4x2dv ::krys::gfx::opengl::glProgramUniformMatrix4x2dv
  #define glProgramUniformMatrix3x4dv ::krys::gfx::opengl::glProgramUniformMatrix3x4dv
  #define glProgramUniformMatrix4x3dv ::krys::gfx::opengl::glProgramUniformMatrix4x3dv
  #define glValidateProgramPipeline ::krys::gfx::opengl::glValidateProgramPipeline
  #define glGetProgramPipelineInfoLog ::krys::gfx::opengl::glGetProgramPipelineInfoLog
  #define glVertexAttribL1d ::krys::gfx::opengl::glVertexAttribL1d
  #define glVertexAttribL2d ::krys::gfx::opengl::glVertexAttribL2d
  #define glVertexAttribL3d ::krys::gfx::opengl::glVertexAttribL3d
  #define glVertexAttribL4d ::krys::gfx::opengl::glVertexAttribL4d
  #define glVertexAttribL1dv ::krys::gfx::opengl::glVertexAttribL1dv
  #define glVertexAttribL2dv ::krys::gfx::opengl::glVertexAttribL2dv
  #define glVertexAttribL3dv ::krys::gfx::opengl::glVertexAttribL3dv
  #define glVertexAttribL4dv ::krys::gfx::opengl::glVertexAttribL4dv
  #define glVertexAttribLPointer ::krys::gfx::opengl::glVertexAttribLPointer
  #define glGetVertexAttribLdv ::krys::gfx::opengl::glGetVertexAttribLdv
  #define glViewportArrayv ::krys::gfx::opengl::glViewportArrayv
  #define glViewportIndexedf ::krys::gfx::opengl::glViewportIndexedf
  #define glViewportIndexedfv ::krys::gfx::opengl::glViewportIndexedfv
  #define glScissorArrayv ::krys::gfx::opengl::glScissorArrayv
  #define glScissorIndexed ::krys::gfx::opengl::glScissorIndexed
  #define glScissorIndexedv ::krys::gfx::opengl::glScissorIndexedv
  #define glDepthRangeArrayv ::krys::gfx::opengl::glDepthRangeArrayv
  #define glDepthRangeIndexed ::krys::gfx::opengl::glDepthRangeIndexed
  #define glGetFloati_v ::krys::gfx::opengl::glGetFloati_v
  #define glGetDoublei_v ::krys::gfx::opengl::glGetDoublei_v
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
  #define glDrawArraysInstancedBaseInstance ::krys::gfx::opengl::glDrawArraysInstancedBaseInstance
  #define glDrawElementsInstancedBaseInstance ::krys::gfx::opengl::glDrawElementsInstancedBaseInstance
  #define glDrawElementsInstancedBaseVertexBaseInstance                                                      \
    ::krys::gfx::opengl::glDrawElementsInstancedBaseVertexBaseInstance
  #define glGetInternalformativ ::krys::gfx::opengl::glGetInternalformativ
  #define glGetActiveAtomicCounterBufferiv ::krys::gfx::opengl::glGetActiveAtomicCounterBufferiv
  #define glBindImageTexture ::krys::gfx::opengl::glBindImageTexture
  #define glMemoryBarrier ::krys::gfx::opengl::glMemoryBarrier
  #define glTexStorage1D ::krys::gfx::opengl::glTexStorage1D
  #define glTexStorage2D ::krys::gfx::opengl::glTexStorage2D
  #define glTexStorage3D ::krys::gfx::opengl::glTexStorage3D
  #define glDrawTransformFeedbackInstanced ::krys::gfx::opengl::glDrawTransformFeedbackInstanced
  #define glDrawTransformFeedbackStreamInstanced ::krys::gfx::opengl::glDrawTransformFeedbackStreamInstanced
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
  #define glClearBufferData ::krys::gfx::opengl::glClearBufferData
  #define glClearBufferSubData ::krys::gfx::opengl::glClearBufferSubData
  #define glDispatchCompute ::krys::gfx::opengl::glDispatchCompute
  #define glDispatchComputeIndirect ::krys::gfx::opengl::glDispatchComputeIndirect
  #define glCopyImageSubData ::krys::gfx::opengl::glCopyImageSubData
  #define glFramebufferParameteri ::krys::gfx::opengl::glFramebufferParameteri
  #define glGetFramebufferParameteriv ::krys::gfx::opengl::glGetFramebufferParameteriv
  #define glGetInternalformati64v ::krys::gfx::opengl::glGetInternalformati64v
  #define glInvalidateTexSubImage ::krys::gfx::opengl::glInvalidateTexSubImage
  #define glInvalidateTexImage ::krys::gfx::opengl::glInvalidateTexImage
  #define glInvalidateBufferSubData ::krys::gfx::opengl::glInvalidateBufferSubData
  #define glInvalidateBufferData ::krys::gfx::opengl::glInvalidateBufferData
  #define glInvalidateFramebuffer ::krys::gfx::opengl::glInvalidateFramebuffer
  #define glInvalidateSubFramebuffer ::krys::gfx::opengl::glInvalidateSubFramebuffer
  #define glMultiDrawArraysIndirect ::krys::gfx::opengl::glMultiDrawArraysIndirect
  #define glMultiDrawElementsIndirect ::krys::gfx::opengl::glMultiDrawElementsIndirect
  #define glGetProgramInterfaceiv ::krys::gfx::opengl::glGetProgramInterfaceiv
  #define glGetProgramResourceIndex ::krys::gfx::opengl::glGetProgramResourceIndex
  #define glGetProgramResourceName ::krys::gfx::opengl::glGetProgramResourceName
  #define glGetProgramResourceiv ::krys::gfx::opengl::glGetProgramResourceiv
  #define glGetProgramResourceLocation ::krys::gfx::opengl::glGetProgramResourceLocation
  #define glGetProgramResourceLocationIndex ::krys::gfx::opengl::glGetProgramResourceLocationIndex
  #define glShaderStorageBlockBinding ::krys::gfx::opengl::glShaderStorageBlockBinding
  #define glTexBufferRange ::krys::gfx::opengl::glTexBufferRange
  #define glTexStorage2DMultisample ::krys::gfx::opengl::glTexStorage2DMultisample
  #define glTexStorage3DMultisample ::krys::gfx::opengl::glTexStorage3DMultisample
  #define glTextureView ::krys::gfx::opengl::glTextureView
  #define glBindVertexBuffer ::krys::gfx::opengl::glBindVertexBuffer
  #define glVertexAttribFormat ::krys::gfx::opengl::glVertexAttribFormat
  #define glVertexAttribIFormat ::krys::gfx::opengl::glVertexAttribIFormat
  #define glVertexAttribLFormat ::krys::gfx::opengl::glVertexAttribLFormat
  #define glVertexAttribBinding ::krys::gfx::opengl::glVertexAttribBinding
  #define glVertexBindingDivisor ::krys::gfx::opengl::glVertexBindingDivisor
  #define glDebugMessageControl ::krys::gfx::opengl::glDebugMessageControl
  #define glDebugMessageInsert ::krys::gfx::opengl::glDebugMessageInsert
  #define glDebugMessageCallback ::krys::gfx::opengl::glDebugMessageCallback
  #define glGetDebugMessageLog ::krys::gfx::opengl::glGetDebugMessageLog
  #define glPushDebugGroup ::krys::gfx::opengl::glPushDebugGroup
  #define glPopDebugGroup ::krys::gfx::opengl::glPopDebugGroup
  #define glObjectLabel ::krys::gfx::opengl::glObjectLabel
  #define glGetObjectLabel ::krys::gfx::opengl::glGetObjectLabel
  #define glObjectPtrLabel ::krys::gfx::opengl::glObjectPtrLabel
  #define glGetObjectPtrLabel ::krys::gfx::opengl::glGetObjectPtrLabel
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
  #define glBufferStorage ::krys::gfx::opengl::glBufferStorage
  #define glClearTexImage ::krys::gfx::opengl::glClearTexImage
  #define glClearTexSubImage ::krys::gfx::opengl::glClearTexSubImage
  #define glBindBuffersBase ::krys::gfx::opengl::glBindBuffersBase
  #define glBindBuffersRange ::krys::gfx::opengl::glBindBuffersRange
  #define glBindTextures ::krys::gfx::opengl::glBindTextures
  #define glBindSamplers ::krys::gfx::opengl::glBindSamplers
  #define glBindImageTextures ::krys::gfx::opengl::glBindImageTextures
  #define glBindVertexBuffers ::krys::gfx::opengl::glBindVertexBuffers
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
  #define glClipControl ::krys::gfx::opengl::glClipControl
  #define glCreateTransformFeedbacks ::krys::gfx::opengl::glCreateTransformFeedbacks
  #define glTransformFeedbackBufferBase ::krys::gfx::opengl::glTransformFeedbackBufferBase
  #define glTransformFeedbackBufferRange ::krys::gfx::opengl::glTransformFeedbackBufferRange
  #define glGetTransformFeedbackiv ::krys::gfx::opengl::glGetTransformFeedbackiv
  #define glGetTransformFeedbacki_v ::krys::gfx::opengl::glGetTransformFeedbacki_v
  #define glGetTransformFeedbacki64_v ::krys::gfx::opengl::glGetTransformFeedbacki64_v
  #define glCreateBuffers ::krys::gfx::opengl::glCreateBuffers
  #define glNamedBufferStorage ::krys::gfx::opengl::glNamedBufferStorage
  #define glNamedBufferData ::krys::gfx::opengl::glNamedBufferData
  #define glNamedBufferSubData ::krys::gfx::opengl::glNamedBufferSubData
  #define glCopyNamedBufferSubData ::krys::gfx::opengl::glCopyNamedBufferSubData
  #define glClearNamedBufferData ::krys::gfx::opengl::glClearNamedBufferData
  #define glClearNamedBufferSubData ::krys::gfx::opengl::glClearNamedBufferSubData
  #define glMapNamedBuffer ::krys::gfx::opengl::glMapNamedBuffer
  #define glMapNamedBufferRange ::krys::gfx::opengl::glMapNamedBufferRange
  #define glUnmapNamedBuffer ::krys::gfx::opengl::glUnmapNamedBuffer
  #define glFlushMappedNamedBufferRange ::krys::gfx::opengl::glFlushMappedNamedBufferRange
  #define glGetNamedBufferParameteriv ::krys::gfx::opengl::glGetNamedBufferParameteriv
  #define glGetNamedBufferParameteri64v ::krys::gfx::opengl::glGetNamedBufferParameteri64v
  #define glGetNamedBufferPointerv ::krys::gfx::opengl::glGetNamedBufferPointerv
  #define glGetNamedBufferSubData ::krys::gfx::opengl::glGetNamedBufferSubData
  #define glCreateFramebuffers ::krys::gfx::opengl::glCreateFramebuffers
  #define glNamedFramebufferRenderbuffer ::krys::gfx::opengl::glNamedFramebufferRenderbuffer
  #define glNamedFramebufferParameteri ::krys::gfx::opengl::glNamedFramebufferParameteri
  #define glNamedFramebufferTexture ::krys::gfx::opengl::glNamedFramebufferTexture
  #define glNamedFramebufferTextureLayer ::krys::gfx::opengl::glNamedFramebufferTextureLayer
  #define glNamedFramebufferDrawBuffer ::krys::gfx::opengl::glNamedFramebufferDrawBuffer
  #define glNamedFramebufferDrawBuffers ::krys::gfx::opengl::glNamedFramebufferDrawBuffers
  #define glNamedFramebufferReadBuffer ::krys::gfx::opengl::glNamedFramebufferReadBuffer
  #define glInvalidateNamedFramebufferData ::krys::gfx::opengl::glInvalidateNamedFramebufferData
  #define glInvalidateNamedFramebufferSubData ::krys::gfx::opengl::glInvalidateNamedFramebufferSubData
  #define glClearNamedFramebufferiv ::krys::gfx::opengl::glClearNamedFramebufferiv
  #define glClearNamedFramebufferuiv ::krys::gfx::opengl::glClearNamedFramebufferuiv
  #define glClearNamedFramebufferfv ::krys::gfx::opengl::glClearNamedFramebufferfv
  #define glClearNamedFramebufferfi ::krys::gfx::opengl::glClearNamedFramebufferfi
  #define glBlitNamedFramebuffer ::krys::gfx::opengl::glBlitNamedFramebuffer
  #define glCheckNamedFramebufferStatus ::krys::gfx::opengl::glCheckNamedFramebufferStatus
  #define glGetNamedFramebufferParameteriv ::krys::gfx::opengl::glGetNamedFramebufferParameteriv
  #define glGetNamedFramebufferAttachmentParameteriv                                                         \
    ::krys::gfx::opengl::glGetNamedFramebufferAttachmentParameteriv
  #define glCreateRenderbuffers ::krys::gfx::opengl::glCreateRenderbuffers
  #define glNamedRenderbufferStorage ::krys::gfx::opengl::glNamedRenderbufferStorage
  #define glNamedRenderbufferStorageMultisample ::krys::gfx::opengl::glNamedRenderbufferStorageMultisample
  #define glGetNamedRenderbufferParameteriv ::krys::gfx::opengl::glGetNamedRenderbufferParameteriv
  #define glCreateTextures ::krys::gfx::opengl::glCreateTextures
  #define glTextureBuffer ::krys::gfx::opengl::glTextureBuffer
  #define glTextureBufferRange ::krys::gfx::opengl::glTextureBufferRange
  #define glTextureStorage1D ::krys::gfx::opengl::glTextureStorage1D
  #define glTextureStorage2D ::krys::gfx::opengl::glTextureStorage2D
  #define glTextureStorage3D ::krys::gfx::opengl::glTextureStorage3D
  #define glTextureStorage2DMultisample ::krys::gfx::opengl::glTextureStorage2DMultisample
  #define glTextureStorage3DMultisample ::krys::gfx::opengl::glTextureStorage3DMultisample
  #define glTextureSubImage1D ::krys::gfx::opengl::glTextureSubImage1D
  #define glTextureSubImage2D ::krys::gfx::opengl::glTextureSubImage2D
  #define glTextureSubImage3D ::krys::gfx::opengl::glTextureSubImage3D
  #define glCompressedTextureSubImage1D ::krys::gfx::opengl::glCompressedTextureSubImage1D
  #define glCompressedTextureSubImage2D ::krys::gfx::opengl::glCompressedTextureSubImage2D
  #define glCompressedTextureSubImage3D ::krys::gfx::opengl::glCompressedTextureSubImage3D
  #define glCopyTextureSubImage1D ::krys::gfx::opengl::glCopyTextureSubImage1D
  #define glCopyTextureSubImage2D ::krys::gfx::opengl::glCopyTextureSubImage2D
  #define glCopyTextureSubImage3D ::krys::gfx::opengl::glCopyTextureSubImage3D
  #define glTextureParameterf ::krys::gfx::opengl::glTextureParameterf
  #define glTextureParameterfv ::krys::gfx::opengl::glTextureParameterfv
  #define glTextureParameteri ::krys::gfx::opengl::glTextureParameteri
  #define glTextureParameterIiv ::krys::gfx::opengl::glTextureParameterIiv
  #define glTextureParameterIuiv ::krys::gfx::opengl::glTextureParameterIuiv
  #define glTextureParameteriv ::krys::gfx::opengl::glTextureParameteriv
  #define glGenerateTextureMipmap ::krys::gfx::opengl::glGenerateTextureMipmap
  #define glBindTextureUnit ::krys::gfx::opengl::glBindTextureUnit
  #define glGetTextureImage ::krys::gfx::opengl::glGetTextureImage
  #define glGetCompressedTextureImage ::krys::gfx::opengl::glGetCompressedTextureImage
  #define glGetTextureLevelParameterfv ::krys::gfx::opengl::glGetTextureLevelParameterfv
  #define glGetTextureLevelParameteriv ::krys::gfx::opengl::glGetTextureLevelParameteriv
  #define glGetTextureParameterfv ::krys::gfx::opengl::glGetTextureParameterfv
  #define glGetTextureParameterIiv ::krys::gfx::opengl::glGetTextureParameterIiv
  #define glGetTextureParameterIuiv ::krys::gfx::opengl::glGetTextureParameterIuiv
  #define glGetTextureParameteriv ::krys::gfx::opengl::glGetTextureParameteriv
  #define glCreateVertexArrays ::krys::gfx::opengl::glCreateVertexArrays
  #define glDisableVertexArrayAttrib ::krys::gfx::opengl::glDisableVertexArrayAttrib
  #define glEnableVertexArrayAttrib ::krys::gfx::opengl::glEnableVertexArrayAttrib
  #define glVertexArrayElementBuffer ::krys::gfx::opengl::glVertexArrayElementBuffer
  #define glVertexArrayVertexBuffer ::krys::gfx::opengl::glVertexArrayVertexBuffer
  #define glVertexArrayVertexBuffers ::krys::gfx::opengl::glVertexArrayVertexBuffers
  #define glVertexArrayAttribBinding ::krys::gfx::opengl::glVertexArrayAttribBinding
  #define glVertexArrayAttribFormat ::krys::gfx::opengl::glVertexArrayAttribFormat
  #define glVertexArrayAttribIFormat ::krys::gfx::opengl::glVertexArrayAttribIFormat
  #define glVertexArrayAttribLFormat ::krys::gfx::opengl::glVertexArrayAttribLFormat
  #define glVertexArrayBindingDivisor ::krys::gfx::opengl::glVertexArrayBindingDivisor
  #define glGetVertexArrayiv ::krys::gfx::opengl::glGetVertexArrayiv
  #define glGetVertexArrayIndexediv ::krys::gfx::opengl::glGetVertexArrayIndexediv
  #define glGetVertexArrayIndexed64iv ::krys::gfx::opengl::glGetVertexArrayIndexed64iv
  #define glCreateSamplers ::krys::gfx::opengl::glCreateSamplers
  #define glCreateProgramPipelines ::krys::gfx::opengl::glCreateProgramPipelines
  #define glCreateQueries ::krys::gfx::opengl::glCreateQueries
  #define glGetQueryBufferObjecti64v ::krys::gfx::opengl::glGetQueryBufferObjecti64v
  #define glGetQueryBufferObjectiv ::krys::gfx::opengl::glGetQueryBufferObjectiv
  #define glGetQueryBufferObjectui64v ::krys::gfx::opengl::glGetQueryBufferObjectui64v
  #define glGetQueryBufferObjectuiv ::krys::gfx::opengl::glGetQueryBufferObjectuiv
  #define glMemoryBarrierByRegion ::krys::gfx::opengl::glMemoryBarrierByRegion
  #define glGetTextureSubImage ::krys::gfx::opengl::glGetTextureSubImage
  #define glGetCompressedTextureSubImage ::krys::gfx::opengl::glGetCompressedTextureSubImage
  #define glGetGraphicsResetStatus ::krys::gfx::opengl::glGetGraphicsResetStatus
  #define glGetnCompressedTexImage ::krys::gfx::opengl::glGetnCompressedTexImage
  #define glGetnTexImage ::krys::gfx::opengl::glGetnTexImage
  #define glGetnUniformdv ::krys::gfx::opengl::glGetnUniformdv
  #define glGetnUniformfv ::krys::gfx::opengl::glGetnUniformfv
  #define glGetnUniformiv ::krys::gfx::opengl::glGetnUniformiv
  #define glGetnUniformuiv ::krys::gfx::opengl::glGetnUniformuiv
  #define glReadnPixels ::krys::gfx::opengl::glReadnPixels
  #define glTextureBarrier ::krys::gfx::opengl::glTextureBarrier
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
  #define glSpecializeShader ::krys::gfx::opengl::glSpecializeShader
  #define glMultiDrawArraysIndirectCount ::krys::gfx::opengl::glMultiDrawArraysIndirectCount
  #define glMultiDrawElementsIndirectCount ::krys::gfx::opengl::glMultiDrawElementsIndirectCount
  #define glPolygonOffsetClamp ::krys::gfx::opengl::glPolygonOffsetClamp
#endif

#pragma endregion

#pragma endregion
}