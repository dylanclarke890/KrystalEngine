#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#undef CreateWindow
#undef LoadImage
#undef min
#undef max

// The bootstrap calls the system WGL functions, so the hook header mustn't replace their names.
#define KRYS_WGL_OVERRIDE_FUNCTIONS 0
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Gfx/OpenGL/GLContext.hpp"
#include "Krystal.Gfx/OpenGL/Hooks/gl.hpp"
#include "Krystal.Gfx/OpenGL/Hooks/wgl.hpp"

#pragma comment(lib, "opengl32.lib")

namespace krys::gfx::opengl
{
  namespace
  {
    constexpr const char *WindowClassName = "krys_gfx_opengl_context";

    KRYS_NODISCARD HWND AsWindow(void *handle) noexcept
    {
      return static_cast<HWND>(handle);
    }

    KRYS_NODISCARD HDC AsSurface(void *handle) noexcept
    {
      return static_cast<HDC>(handle);
    }

    KRYS_NODISCARD bool RegisterWindowClass(HINSTANCE instance) noexcept
    {
      WNDCLASSA windowClass {};
      windowClass.lpfnWndProc = ::DefWindowProcA;
      windowClass.hInstance = instance;
      windowClass.lpszClassName = WindowClassName;
      windowClass.style = CS_OWNDC;
      return ::RegisterClassA(&windowClass) != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }

    KRYS_NODISCARD HWND CreateHiddenWindow(HINSTANCE instance) noexcept
    {
      return ::CreateWindowA(WindowClassName, "Krystal OpenGL context", WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0,
                             0, 1, 1, nullptr, nullptr, instance, nullptr);
    }

    /// @brief Loads the WGL and GL entry points through a throwaway legacy context, which is the only way to
    /// reach `wglChoosePixelFormatARB` and `wglCreateContextAttribsARB`.
    KRYS_NODISCARD Expected<void> LoadEntryPoints(HINSTANCE instance) noexcept
    {
      HWND window = CreateHiddenWindow(instance);
      if (window == nullptr)
      {
        return Unexpected<string>("Failed to create the window for loading OpenGL.");
      }

      HDC surface = ::GetDC(window);
      PIXELFORMATDESCRIPTOR pixelFormatDesc {};
      pixelFormatDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      pixelFormatDesc.nVersion = 1;
      pixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
      pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
      pixelFormatDesc.cColorBits = 24;
      pixelFormatDesc.cAlphaBits = 8;

      Expected<void> result {};
      int pixelFormat = ::ChoosePixelFormat(surface, &pixelFormatDesc);
      HGLRC context = nullptr;
      if (pixelFormat == 0 || !::SetPixelFormat(surface, pixelFormat, &pixelFormatDesc))
      {
        result = Unexpected<string>("Failed to set a pixel format for loading OpenGL.");
      }
      else if ((context = ::wglCreateContext(surface)) == nullptr || !::wglMakeCurrent(surface, context))
      {
        result = Unexpected<string>("Failed to create a context for loading OpenGL.");
      }
      else if (!InitialiseWGLHooks(surface) || !InitialiseGLHooks())
      {
        result = Unexpected<string>("Failed to load the OpenGL entry points.");
      }

      ::wglMakeCurrent(nullptr, nullptr);
      if (context != nullptr)
      {
        ::wglDeleteContext(context);
      }
      ::ReleaseDC(window, surface);
      ::DestroyWindow(window);
      return result;
    }

    KRYS_NODISCARD Array<int, 21> PixelFormatAttributes(bool srgb) noexcept
    {
      return {
        WGL_DRAW_TO_WINDOW_ARB,
        GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,
        GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,
        GL_TRUE,
        WGL_PIXEL_TYPE_ARB,
        WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,
        24,
        WGL_ALPHA_BITS_ARB,
        8,
        WGL_DEPTH_BITS_ARB,
        0,
        WGL_STENCIL_BITS_ARB,
        0,
        srgb ? WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB : 0,
        srgb ? GL_TRUE : 0,
        0,
        0,
        0,
      };
    }
  }

  Expected<UniquePtr<GLContext>> GLContext::Create(bool debug) noexcept
  {
    auto context = CreateUnique<GLContext>();
    if (auto result = context->Initialise(debug); !result)
    {
      return Unexpected<string>(result.error());
    }
    return context;
  }

  Expected<void> GLContext::Initialise(bool debug) noexcept
  {
    HINSTANCE instance = ::GetModuleHandleA(nullptr);
    if (!RegisterWindowClass(instance))
    {
      return Unexpected<string>("Failed to register the OpenGL context's window class.");
    }

    if (auto result = LoadEntryPoints(instance); !result)
    {
      return result;
    }

    _hiddenWindow = CreateHiddenWindow(instance);
    if (_hiddenWindow == nullptr)
    {
      return Unexpected<string>("Failed to create the OpenGL context's window.");
    }
    _hiddenSurface = ::GetDC(AsWindow(_hiddenWindow));

    _supportsSrgb = IsWGLExtensionSupported("WGL_ARB_framebuffer_sRGB")
                    || IsWGLExtensionSupported("WGL_EXT_framebuffer_sRGB");
    auto attributes = PixelFormatAttributes(_supportsSrgb);
    UINT formatCount = 0u;
    if (!opengl::wglChoosePixelFormatARB(AsSurface(_hiddenSurface), attributes.data(), nullptr, 1u,
                                         &_pixelFormat, &formatCount)
        || formatCount == 0u)
    {
      return Unexpected<string>("No pixel format supports OpenGL rendering to a window.");
    }

    PIXELFORMATDESCRIPTOR pixelFormatDesc {};
    ::DescribePixelFormat(AsSurface(_hiddenSurface), _pixelFormat, sizeof(PIXELFORMATDESCRIPTOR),
                          &pixelFormatDesc);
    if (!::SetPixelFormat(AsSurface(_hiddenSurface), _pixelFormat, &pixelFormatDesc))
    {
      return Unexpected<string>("Failed to set the OpenGL context's pixel format.");
    }

    const int contextAttributes[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB,
      4,
      WGL_CONTEXT_MINOR_VERSION_ARB,
      5,
      WGL_CONTEXT_PROFILE_MASK_ARB,
      WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      WGL_CONTEXT_FLAGS_ARB,
      WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | (debug ? WGL_CONTEXT_DEBUG_BIT_ARB : 0),
      0,
    };
    _context = opengl::wglCreateContextAttribsARB(AsSurface(_hiddenSurface), nullptr, contextAttributes);
    if (_context == nullptr)
    {
      return Unexpected<string>("Failed to create an OpenGL 4.5 core context.");
    }

    MakeCurrent({});
    // Entry points can differ between contexts; reload them for the real one.
    if (!InitialiseGLHooks())
    {
      return Unexpected<string>("Failed to load the OpenGL entry points.");
    }
    return {};
  }

  GLContext::~GLContext() noexcept
  {
    if (_context != nullptr)
    {
      KRYS_MAYBE_UNUSED auto released = opengl::wglMakeCurrent(nullptr, nullptr);
      KRYS_MAYBE_UNUSED auto deleted = opengl::wglDeleteContext(static_cast<HGLRC>(_context));
    }
    if (_hiddenWindow != nullptr)
    {
      ::ReleaseDC(AsWindow(_hiddenWindow), AsSurface(_hiddenSurface));
      ::DestroyWindow(AsWindow(_hiddenWindow));
    }
  }

  Expected<NativeHandle> GLContext::AttachWindow(NativeHandle window) noexcept
  {
    HWND hwnd = window.As<HWND>();
    HDC surface = ::GetDC(hwnd);
    if (surface == nullptr)
    {
      return Unexpected<string>("Failed to get the window's device context.");
    }

    // A window's pixel format can only be set once; one set by an earlier swapchain must match.
    int existing = ::GetPixelFormat(surface);
    if (existing == 0)
    {
      PIXELFORMATDESCRIPTOR pixelFormatDesc {};
      ::DescribePixelFormat(surface, _pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDesc);
      if (!::SetPixelFormat(surface, _pixelFormat, &pixelFormatDesc))
      {
        ::ReleaseDC(hwnd, surface);
        return Unexpected<string>("Failed to set the window's pixel format.");
      }
    }
    else if (existing != _pixelFormat)
    {
      ::ReleaseDC(hwnd, surface);
      return Unexpected<string>("The window already has an incompatible pixel format.");
    }
    return NativeHandle {surface};
  }

  void GLContext::DetachWindow(NativeHandle window, NativeHandle surface) noexcept
  {
    if (_currentSurface == surface.As<void *>())
    {
      MakeCurrent({});
    }
    ::ReleaseDC(window.As<HWND>(), surface.As<HDC>());
  }

  void GLContext::MakeCurrent(NativeHandle surface) noexcept
  {
    void *target = surface.IsValid() ? surface.As<void *>() : _hiddenSurface;
    if (target == _currentSurface)
    {
      return;
    }
    auto result = opengl::wglMakeCurrent(AsSurface(target), static_cast<HGLRC>(_context));
    krys_assert_msg(result, "Failed to make the OpenGL context current.");
    _currentSurface = target;
  }

  void GLContext::SwapBuffers(NativeHandle surface, bool vsync) noexcept
  {
    MakeCurrent(surface);
    int interval = vsync ? 1 : 0;
    if (interval != _swapInterval && IsWGLExtensionSupported("WGL_EXT_swap_control"))
    {
      KRYS_MAYBE_UNUSED auto result = opengl::wglSwapIntervalEXT(interval);
      _swapInterval = interval;
    }
    auto result = opengl::wglSwapBuffers(surface.As<HDC>());
    krys_assert_msg(result, "Failed to present.");
  }
}
