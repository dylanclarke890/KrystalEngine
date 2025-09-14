#pragma once

#include "Krystal.Core/Core.hpp"
#include <tuple>

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #define NOGDICAPMASKS
  #define NOVIRTUALKEYCODES
  #define NOWINMESSAGES
  #define NOWINSTYLES
  #define NOSYSMETRICS
  #define NOMENUS
  #define NOICONS
  #define NOKEYSTATES
  #define NOSYSCOMMANDS
  #define NORASTEROPS
  #define NOSHOWWINDOW
  #define OEMRESOURCE
  #define NOATOM
  #define NOCLIPBOARD
  #define NOCOLOR
  #define NOCTLMGR
  #define NODRAWTEXT
  #define NOKERNEL
  #define NOUSER
  #define NONLS
  #define NOMB
  #define NOMEMMGR
  #define NOMETAFILE
  #define NOMINMAX
  #define NOMSG
  #define NOOPENFILE
  #define NOSCROLL
  #define NOSERVICE
  #define NOSOUND
  #define NOTEXTMETRIC
  #define NOWH
  #define NOWINOFFSETS
  #define NOCOMM
  #define NOKANJI
  #define NOHELP
  #define NOPROFILER
  #define NODEFERWINDOWPOS
  #define NOMCX
  // #define NOGDI - we need GDI for pixel format functions
  #include <windows.h>
  #undef near
  #undef far
#endif

// #ifndef KRYS_GL_OVERRIDE_FUNCTIONS
//   #define KRYS_GL_OVERRIDE_FUNCTIONS 0
// #endif

#ifndef KRYS_WGL_OVERRIDE_FUNCTIONS
  #define KRYS_WGL_OVERRIDE_FUNCTIONS 1
#endif

#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"

#pragma region WGL Extensions Header

/*
  WGL extensions are defined in wglext.h, which is included below. It uses a series of defines to add new
  function pointer types and symbols for WGL functions, only including them if they are not already defined.
  We can remove extensions that we do not support or need in our application by defining them before
  including wglext.h. This doesn't have any real effect on the code, but it allows us to
  stop the irrelevant symbols from polluting our namespace.
 */

#ifndef KRYS_WGL_INCLUDE_ALL_EXTENSIONS
  #define WGL_ARB_buffer_region 1
  #define WGL_ARB_context_flush_control 1
  #define WGL_ARB_create_context_robustness 1
  #define WGL_ARB_make_current_read 1
  #define WGL_ARB_pbuffer 1
  #define WGL_ARB_render_texture 1
  #define WGL_ARB_robustness_application_isolation 1
  #define WGL_ARB_robustness_share_group_isolation 1
  #define WGL_3DFX_multisample 1
  #define WGL_3DL_stereo_control 1
  #define WGL_AMD_gpu_association 1
  #define WGL_ATI_pixel_format_float 1
  #define WGL_ATI_render_texture_rectangle 1
  #define WGL_EXT_colorspace 1
  #define WGL_EXT_create_context_es2_profile 1
  #define WGL_EXT_create_context_es_profile 1
  #define WGL_EXT_depth_float 1
  #define WGL_EXT_display_color_table 1
  #define WGL_EXT_framebuffer_sRGB 1
  #define WGL_EXT_make_current_read 1
  #define WGL_EXT_multisample 1
  #define WGL_EXT_pbuffer 1
  #define WGL_EXT_pixel_format 1
  #define WGL_EXT_pixel_format_packed_float 1
  #define WGL_I3D_digital_video_control 1
  #define WGL_I3D_gamma 1
  #define WGL_I3D_genlock 1
  #define WGL_I3D_image_buffer 1
  #define WGL_I3D_swap_frame_lock 1
  #define WGL_I3D_swap_frame_usage 1
  #define WGL_NV_DX_interop 1
  #define WGL_NV_DX_interop2 1
  #define WGL_NV_copy_image 1
  #define WGL_NV_delay_before_swap 1
  #define WGL_NV_float_buffer 1
  #define WGL_NV_gpu_affinity 1
  #define WGL_NV_multigpu_context 1
  #define WGL_NV_multisample_coverage 1
  #define WGL_NV_present_video 1
  #define WGL_NV_render_depth_texture 1
  #define WGL_NV_render_texture_rectangle 1
  #define WGL_NV_swap_group 1
  #define WGL_NV_vertex_array_range 1
  #define WGL_NV_video_capture 1
  #define WGL_NV_video_output 1
  #define WGL_OML_sync_control 1
#endif

#include "Krystal.Gfx.OpenGL/Official/wglext.h"

#pragma endregion

namespace Krys::Gfx::OpenGL
{
  /// @brief Initializes WGL hooks for OpenGL context management.
  NO_DISCARD bool InitialiseWGLHooks(HDC deviceContext) noexcept;

  /// @brief Checks if a specific WGL extension is supported.
  NO_DISCARD bool IsWGLExtensionSupported(const char *extension) noexcept;
}

// WGL hook declarations
namespace Krys::Gfx::OpenGL
{
#pragma region WGL Core Functions

  /// @brief Gets the current draw device context for the calling thread.
  NO_DISCARD HDC wglGetCurrentDC() noexcept;

  /// @brief Gets the current OpenGL context associated with the calling thread.
  NO_DISCARD HGLRC wglGetCurrentContext() noexcept;

  /// @brief Makes the specified GL context current for the specified device context.
  NO_DISCARD BOOL wglMakeCurrent(HDC hdc, HGLRC hglrc) noexcept;

  /// @brief Destroys the specified GL context and frees its resources.
  NO_DISCARD BOOL wglDeleteContext(HGLRC hglrc) noexcept;

  /// @brief Swaps the front and back buffers of the specified device context.
  NO_DISCARD BOOL wglSwapBuffers(HDC hdc) noexcept;

#if KRYS_WGL_OVERRIDE_FUNCTIONS
  /// @brief Gets the current draw device context for the calling thread.
  #define wglGetCurrentDC ::Krys::Gfx::OpenGL::wglGetCurrentDC

  /// @brief Gets the current OpenGL context associated with the calling thread.
  #define wglGetCurrentContext ::Krys::Gfx::OpenGL::wglGetCurrentContext

  /// @brief Makes the specified GL context current for the specified device context.
  #define wglMakeCurrent ::Krys::Gfx::OpenGL::wglMakeCurrent

  /// @brief Destroys the specified GL context and frees its resources.
  #define wglDeleteContext ::Krys::Gfx::OpenGL::wglDeleteContext

  /// @brief Swaps the front and back buffers of the specified device context.
  #define wglSwapBuffers ::Krys::Gfx::OpenGL::wglSwapBuffers
#endif

#pragma endregion

#pragma region WGL_ARB_extensions_string

  /// @brief Gets a space delimited string containing all available WGL extensions.
  ///
  /// @extension WGL_ARB_extensions_string
  NO_DISCARD const char *wglGetExtensionsStringARB(HDC dc) noexcept;

#if KRYS_WGL_OVERRIDE_FUNCTIONS
  /// @brief Gets a space delimited string containing all available WGL extensions.
  ///
  /// @extension WGL_ARB_extensions_string
  #define wglGetExtensionsStringARB ::Krys::Gfx::OpenGL::wglGetExtensionsStringARB
#endif

#pragma endregion

#pragma region WGL_ARB_create_context

  /// @brief Creates an OpenGL context with specified attributes.
  ///
  /// @extension WGL_ARB_create_context
  NO_DISCARD HGLRC wglCreateContextAttribsARB(HDC dc, HGLRC shareContext, const int *attribs) noexcept;

#if KRYS_WGL_OVERRIDE_FUNCTIONS
  /// @brief Creates an OpenGL context with specified attributes.
  ///
  /// @extension WGL_ARB_create_context
  #define wglCreateContextAttribsARB ::Krys::Gfx::OpenGL::wglCreateContextAttribsARB
#endif

#pragma endregion

#pragma region WGL_ARB_pixel_format

  /// @brief Gets a list of pixel formats that match the specified attributes.
  ///
  /// @extension WGL_ARB_pixel_format
  NO_DISCARD BOOL wglGetPixelFormatAttribivARB(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes,
                                               const int *piAttributes, int *piValues) noexcept;

  /// @brief Gets the values of pixel format attributes as floating-point numbers.
  ///
  /// @extension WGL_ARB_pixel_format
  NO_DISCARD BOOL wglGetPixelFormatAttribfvARB(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes,
                                               const int *piAttributes, float *pfValues) noexcept;

  /// @brief Chooses a pixel format based on specified attributes and returns the number of formats found.
  ///
  /// @extension WGL_ARB_pixel_format
  NO_DISCARD BOOL wglChoosePixelFormatARB(HDC hdc, const int *piAttribIList, const float *pfAttribFList,
                                          UINT nMaxFormats, int *piFormats, UINT *nNumFormats) noexcept;

#if KRYS_WGL_OVERRIDE_FUNCTIONS
  /// @brief Gets a list of pixel formats that match the specified attributes.
  ///
  /// @extension WGL_ARB_pixel_format
  #define wglGetPixelFormatAttribivARB ::Krys::Gfx::OpenGL::wglGetPixelFormatAttribivARB

  /// @brief Gets the values of pixel format attributes as floating-point numbers.
  ///
  /// @extension WGL_ARB_pixel_format
  #define wglGetPixelFormatAttribfvARB ::Krys::Gfx::OpenGL::wglGetPixelFormatAttribfvARB

  /// @brief Chooses a pixel format based on specified attributes and returns the number of formats found.
  ///
  /// @extension WGL_ARB_pixel_format
  #define wglChoosePixelFormatARB ::Krys::Gfx::OpenGL::wglChoosePixelFormatARB
#endif

#pragma endregion

#pragma region WGL_EXT_swap_control

  /// @brief Sets the swap interval for vertical synchronization. If interval is 0, it disables VSync.
  /// If interval is 1, it enables VSync. If "WGL_EXT_swap_control_tear" is supported, interval can be set to
  /// -1.
  /// @extension WGL_EXT_swap_control
  NO_DISCARD BOOL wglSwapIntervalEXT(int interval) noexcept;

  /// @brief Gets the current swap interval for vertical synchronization.
  ///
  /// @extension WGL_EXT_swap_control
  NO_DISCARD int wglGetSwapIntervalEXT() noexcept;

#if KRYS_WGL_OVERRIDE_FUNCTIONS
  /// @brief Sets the swap interval for vertical synchronization. If interval is 0, it disables VSync.
  /// If interval is 1, it enables VSync. If "WGL_EXT_swap_control_tear" is supported, interval can be set to
  /// -1.
  /// @extension WGL_EXT_swap_control
  #define wglSwapIntervalEXT ::Krys::Gfx::OpenGL::wglSwapIntervalEXT

  /// @brief Gets the current swap interval for vertical synchronization.
  ///
  /// @extension WGL_EXT_swap_control
  #define wglGetSwapIntervalEXT ::Krys::Gfx::OpenGL::wglGetSwapIntervalEXT
#endif

#pragma endregion
}
