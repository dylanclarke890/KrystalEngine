#define KRYS_WGL_OVERRIDE_FUNCTIONS 0
#include "Krystal.Gfx.OpenGL/Hooks/wgl.hpp"
#include "Krystal.Lib/String/StringUtils.hpp"
#include <algorithm>
#include <cassert>

namespace
{
  using ::Krys::string;
  template <typename T>
  using List = ::Krys::List<T>;

  using PFNWGLGETCURRENTDCPROC = HDC(WINAPI *)(void);
  using PFNWGLGETCURRENTCONTEXTPROC = HGLRC(WINAPI *)(void);
  using PFNWGLMAKECURRENTPROC = BOOL(WINAPI *)(HDC, HGLRC);
  using PFNWGLDELETECONTEXTPROC = BOOL(WINAPI *)(HGLRC);
  using PFNWGLSWAPBUFFERS = BOOL(WINAPI *)(HDC);

  struct WGL
  {
    // Core functions
    PFNWGLGETCURRENTDCPROC GetCurrentDC;
    PFNWGLGETCURRENTCONTEXTPROC GetCurrentContext;
    PFNWGLMAKECURRENTPROC MakeCurrent;
    PFNWGLDELETECONTEXTPROC DeleteContext;
    PFNWGLSWAPBUFFERS SwapBuffers;

    // WGL_ARB_extensions_string
    PFNWGLGETEXTENSIONSSTRINGARBPROC GetExtensionsStringARB;

    // WGL_ARB_create_context
    PFNWGLCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB;

    // WGL_ARB_pixel_format
    PFNWGLGETPIXELFORMATATTRIBIVARBPROC GetPixelFormatAttribivARB;
    PFNWGLGETPIXELFORMATATTRIBFVARBPROC GetPixelFormatAttribfvARB;
    PFNWGLCHOOSEPIXELFORMATARBPROC ChoosePixelFormatARB;

    // WGL_EXT_swap_control
    PFNWGLSWAPINTERVALEXTPROC SwapIntervalEXT;
    PFNWGLGETSWAPINTERVALEXTPROC GetSwapIntervalEXT;
  };

  static WGL wgl = {nullptr};

  static void *GetFunctionAddress(const char *name, const char *fallbackModule = "opengl32.dll") noexcept
  {
    void *p = (void *)::wglGetProcAddress(name);
    if (p == 0 || (p == (void *)0x1) || (p == (void *)0x2) || (p == (void *)0x3) || (p == (void *)-1))
    {
      HMODULE module = LoadLibraryA(fallbackModule);
      if (module)
      {
        p = (void *)GetProcAddress(module, name);
      }
      else
      {
        assert(false && "Failed to load OpenGL module.");
        p = nullptr;
      }
    }

    return p;
  }

  static List<string> RequiredExtensions {"WGL_ARB_extensions_string", "WGL_ARB_create_context",
                                          "WGL_ARB_pixel_format", "WGL_EXT_swap_control"};

  // Will be fetched when `InitialiseWGLHooks` is called.
  static List<string> SupportedExtensions {};

  static bool RequiredExtensionsAreSupported() noexcept
  {
    for (const auto &required : RequiredExtensions)
    {
      if (std::find(SupportedExtensions.begin(), SupportedExtensions.end(), required)
          == SupportedExtensions.end())
      {
        return false; // Required extension not found
      }
    }
    return true;
  }
}

namespace
{
#define LoadWGLFunc(name, funcPtrTypedef)                                                                    \
  wgl.name = (funcPtrTypedef)GetFunctionAddress(STRINGIFY(CONCAT(wgl, name)))

#define LoadExtension(name) CONCAT(LoadExtension_, name)()

  static void LoadCoreFunctions() noexcept
  {
    LoadWGLFunc(GetCurrentDC, PFNWGLGETCURRENTDCPROC);
    LoadWGLFunc(GetCurrentContext, PFNWGLGETCURRENTCONTEXTPROC);
    LoadWGLFunc(MakeCurrent, PFNWGLMAKECURRENTPROC);
    LoadWGLFunc(DeleteContext, PFNWGLDELETECONTEXTPROC);
    // SwapBuffers is not a WGL function, but it's what we use to swap buffers in OpenGL on Windows so it may
    // as well be here.
    wgl.SwapBuffers = (PFNWGLSWAPBUFFERS)GetFunctionAddress("SwapBuffers", "Gdi32.dll");
  }

  static void LoadExtension_ARB_extensions_string() noexcept
  {
    // only adds wglGetExtensionsStringARB.
    LoadWGLFunc(GetExtensionsStringARB, PFNWGLGETEXTENSIONSSTRINGARBPROC);
  }

  static void LoadExtension_ARB_create_context() noexcept
  {
    // only adds wglCreateContextAttribsARB (along with some new defines).
    LoadWGLFunc(CreateContextAttribsARB, PFNWGLCREATECONTEXTATTRIBSARBPROC);
  }

  static void LoadExtension_ARB_pixel_format() noexcept
  {
    LoadWGLFunc(GetPixelFormatAttribivARB, PFNWGLGETPIXELFORMATATTRIBIVARBPROC);
    LoadWGLFunc(GetPixelFormatAttribfvARB, PFNWGLGETPIXELFORMATATTRIBFVARBPROC);
    LoadWGLFunc(ChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC);
  }

  static void LoadExtension_ARB_swap_control() noexcept
  {
    wgl.SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)GetFunctionAddress(STRINGIFY(CONCAT(wgl, SwapIntervalEXT)));
    LoadWGLFunc(GetSwapIntervalEXT, PFNWGLGETSWAPINTERVALEXTPROC);
  }
}

namespace Krys::Gfx::OpenGL
{
  bool InitialiseWGLHooks(HDC deviceContext) noexcept
  {
    // Wrap core WGL functions.
    LoadCoreFunctions();

    // Load the extension for getting available WGL extensions.
    LoadExtension(ARB_extensions_string);

    // Ensure we have the WGL extensions we need.
    SupportedExtensions = StringUtils::Split(wglGetExtensionsStringARB(deviceContext), " ");
    if (!RequiredExtensionsAreSupported())
    {
      assert(false && "Required WGL extensions are not supported by the system.");
      return false;
    }

    LoadExtension(ARB_create_context);
    LoadExtension(ARB_pixel_format);
    LoadExtension(ARB_swap_control);

    return true;
  }

  bool IsWGLExtensionSupported(const char *extension) noexcept
  {
    return std::find(SupportedExtensions.begin(), SupportedExtensions.end(), extension)
           != SupportedExtensions.end();
  }
}

// WGL hook definitions
namespace Krys::Gfx::OpenGL
{
#pragma region WGL Core Functions

  HDC wglGetCurrentDC() noexcept
  {
    return wgl.GetCurrentDC();
  }

  HGLRC wglGetCurrentContext() noexcept
  {
    return wgl.GetCurrentContext();
  }

  BOOL wglMakeCurrent(HDC hdc, HGLRC hglrc) noexcept
  {
    return wgl.MakeCurrent(hdc, hglrc);
  }

  BOOL wglDeleteContext(HGLRC hglrc) noexcept
  {
    return wgl.DeleteContext(hglrc);
  }

  BOOL wglSwapBuffers(HDC hdc) noexcept
  {
    return wgl.SwapBuffers(hdc);
  }

#pragma endregion

#pragma region WGL_ARB_extensions_string

  const char *wglGetExtensionsStringARB(HDC dc) noexcept
  {
    return wgl.GetExtensionsStringARB(dc);
  }

#pragma endregion

#pragma region WGL_ARB_create_context

  HGLRC wglCreateContextAttribsARB(HDC hdc, HGLRC hShareContext, const int *attribs) noexcept
  {
    return wgl.CreateContextAttribsARB(hdc, hShareContext, attribs);
  }

#pragma endregion

#pragma region WGL_ARB_pixel_format

  BOOL wglGetPixelFormatAttribivARB(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes,
                                    const int *piAttributes, int *piValues) noexcept
  {
    return wgl.GetPixelFormatAttribivARB(hdc, iPixelFormat, iLayerPlane, nAttributes, piAttributes, piValues);
  }

  BOOL wglGetPixelFormatAttribfvARB(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes,
                                    const int *piAttributes, float *pfValues) noexcept
  {
    return wgl.GetPixelFormatAttribfvARB(hdc, iPixelFormat, iLayerPlane, nAttributes, piAttributes, pfValues);
  }

  BOOL wglChoosePixelFormatARB(HDC hdc, const int *piAttribIList, const float *pfAttribFList,
                               UINT nMaxFormats, int *piFormats, UINT *nNumFormats) noexcept
  {
    return wgl.ChoosePixelFormatARB(hdc, piAttribIList, pfAttribFList, nMaxFormats, piFormats, nNumFormats);
  }

#pragma endregion

#pragma region WGL_EXT_swap_control

  BOOL wglSwapIntervalEXT(int interval) noexcept
  {
    return wgl.SwapIntervalEXT(interval);
  }

  int wglGetSwapIntervalEXT() noexcept
  {
    return wgl.GetSwapIntervalEXT();
  }

#pragma endregion
}

#undef LoadExtension
#undef LoadWGLFunc