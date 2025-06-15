#include <cassert>
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif

namespace Krys::Gfx::OpenGL
{
  bool InitialiseWGLHooks(HDC deviceContext) noexcept;
  bool InitialiseGLHooks() noexcept;
}

// Declare this function first as we don't care about hooking these function calls, only ones that are used
// for actual OpenGL rendering.
namespace
{
  /// @brief Creates a dummy window and context to initialize WGL extensions.
  static void InitialiseExtensions(const HMODULE instance) noexcept
  {
    WNDCLASSA _class {};
    _class.lpfnWndProc = ::DefWindowProcA;
    _class.hInstance = instance;
    _class.lpszClassName = "wgl_extension_loader_class";
    _class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

    {
      auto result = ::RegisterClassA(&_class);
      assert(result);
    }

    HWND handle = ::CreateWindowA(_class.lpszClassName, "Fake Window", WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0,
                                  0, 1, 1, NULL, NULL, instance, NULL);
    assert(handle);

    HDC deviceContext = ::GetDC(handle);
    assert(deviceContext);

    {
      // specify an arbitrary PFD with OpenGL capabilities
      PIXELFORMATDESCRIPTOR pixelFormatDesc {};
      pixelFormatDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      pixelFormatDesc.nVersion = 1;
      pixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
      pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
      pixelFormatDesc.cColorBits = 24;
      pixelFormatDesc.cAlphaBits = 8;
      pixelFormatDesc.cDepthBits = 24;
      pixelFormatDesc.cStencilBits = 8;

      int pixelFormat = ::ChoosePixelFormat(deviceContext, &pixelFormatDesc);
      assert(pixelFormat != 0);

      auto result = ::SetPixelFormat(deviceContext, pixelFormat, &pixelFormatDesc);
      assert(result);
    }

    HGLRC context = ::wglCreateContext(deviceContext);

    {
      assert(context);
      auto result = ::wglMakeCurrent(deviceContext, context);
      assert(result);
    }

    {
      auto result = Krys::Gfx::OpenGL::InitialiseWGLHooks(deviceContext);
      assert(result);

      result = Krys::Gfx::OpenGL::InitialiseGLHooks();
      assert(result);
    }

    {
      auto result = ::wglMakeCurrent(NULL, NULL);
      assert(result);

      result = ::wglDeleteContext(context);
      assert(result);

      result = ::ReleaseDC(handle, deviceContext);
      assert(result);

      result = ::DestroyWindow(handle);
      assert(result);

      result = ::UnregisterClassA(_class.lpszClassName, instance);
      assert(result);
    }
  }
}

#include "Krystal.Core/Detection.hpp"
#include "Krystal.Gfx.OpenGL/Context.hpp"
#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.Gfx.OpenGL/Hooks/wgl.hpp"
#include "Krystal.Gfx.OpenGL/Win32/Impl.hpp"

namespace Krys::Gfx
{
  void OpenGLContext::Impl::Initialise() noexcept
  {
    const auto instance = ::GetModuleHandleA(NULL);
    assert(instance);

    InitialiseExtensions(instance);

    _deviceContext = ::GetDC(_handle);
    assert(_deviceContext);

    SetupPixelFormat();
    SetupContext();
  }

  void OpenGLContext::Impl::SetupPixelFormat() const noexcept
  {
    using namespace Krys::Gfx::OpenGL;

    auto attributes = GetPixelFormatAttributes();
    int pixelFormat = 0;
    {
      UINT numFormats = 0;
      auto result =
        wglChoosePixelFormatARB(_deviceContext, attributes.data(), NULL, 1, &pixelFormat, &numFormats);
      assert(result && numFormats > 0 && pixelFormat != 0);
    }

    PIXELFORMATDESCRIPTOR pixelFormatDesc {};
    {
      auto result =
        ::DescribePixelFormat(_deviceContext, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDesc);
      assert(result);
    }

    {
      auto result = ::SetPixelFormat(_deviceContext, pixelFormat, &pixelFormatDesc);
      assert(result);
    }
  }

  List<int> OpenGLContext::Impl::GetPixelFormatAttributes() const noexcept
  {
    List<int> attributes;

    // Window draw support
    {
      attributes.push_back(WGL_DRAW_TO_WINDOW_ARB);
      attributes.push_back(GL_TRUE);
    }

    // Double buffering support
    {
      attributes.push_back(WGL_DOUBLE_BUFFER_ARB);
      attributes.push_back(GL_TRUE);
    }

    // OpenGL support
    {
      attributes.push_back(WGL_SUPPORT_OPENGL_ARB);
      attributes.push_back(GL_TRUE);
    }

    // Pixel format
    {
      attributes.push_back(WGL_PIXEL_TYPE_ARB);
      attributes.push_back(WGL_TYPE_RGBA_ARB);

      attributes.push_back(WGL_COLOR_BITS_ARB);
      attributes.push_back(24);

      attributes.push_back(WGL_ALPHA_BITS_ARB);
      attributes.push_back(8);

      attributes.push_back(WGL_DEPTH_BITS_ARB);
      attributes.push_back(24);

      attributes.push_back(WGL_STENCIL_BITS_ARB);
      attributes.push_back(8);
    }

    // TODO: this should be configurable
    bool srgbCapable = false;
    if (srgbCapable && OpenGL::IsWGLExtensionSupported("WGL_ARB_framebuffer_sRGB"))
    {
      attributes.push_back(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB);
      attributes.push_back(GL_TRUE);
    }

    // TODO: this should be configurable
    bool multisampleCapable = false;
    if (multisampleCapable && OpenGL::IsWGLExtensionSupported("WGL_ARB_multisample"))
    {
      attributes.push_back(WGL_SAMPLE_BUFFERS_ARB);
      attributes.push_back(1); // Enable multisampling
      // TODO: this should be configurable
      attributes.push_back(WGL_SAMPLES_ARB);
      attributes.push_back(4); // 4x MSAA
    }

    // End of attributes
    attributes.push_back(0);

    return attributes;
  }

  void OpenGLContext::Impl::SetupContext() noexcept
  {
    auto attributes = GetContextAttributes();
    _renderingContext = wglCreateContextAttribsARB(_deviceContext, 0, attributes.data());
    assert(_renderingContext);

    {
      auto result = wglMakeCurrent(_deviceContext, _renderingContext);
      assert(result);
    }
  }

  List<int> OpenGLContext::Impl::GetContextAttributes() const noexcept
  {
    List<int> attributes;
    attributes.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB);
    attributes.push_back(4);

    attributes.push_back(WGL_CONTEXT_MINOR_VERSION_ARB);
    attributes.push_back(0);

    if (OpenGL::IsWGLExtensionSupported("WGL_ARB_create_context_profile"))
    {
      attributes.push_back(WGL_CONTEXT_PROFILE_MASK_ARB);
      attributes.push_back(WGL_CONTEXT_CORE_PROFILE_BIT_ARB);
    }

    // TODO: this should be configurable
    bool disableErrorChecking = false;
    if (disableErrorChecking && OpenGL::IsWGLExtensionSupported("WGL_ARB_create_context_no_error"))
    {
      attributes.push_back(WGL_CONTEXT_OPENGL_NO_ERROR_ARB);
      attributes.push_back(GL_TRUE);
    }

    // Context flags
    {
      attributes.push_back(WGL_CONTEXT_FLAGS_ARB);
      int ContextFlags = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
#ifdef KRYS_DEBUG
      ContextFlags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif
      attributes.push_back(ContextFlags);
    }

    attributes.push_back(0); // End of attributes

    return attributes;
  }

  void OpenGLContext::Impl::Present() const noexcept
  {
    assert(_deviceContext && _renderingContext);
    auto result = wglSwapBuffers(_deviceContext);
    assert(result);
  }
}