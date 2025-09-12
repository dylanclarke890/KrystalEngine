#include <cassert>
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #undef CreateWindow
  #undef LoadImage
  #undef min
  #undef max
#endif

#include <stdexcept>

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
  static void InitialiseExtensions(const HMODULE instance)
  {
    WNDCLASSA _class {};
    _class.lpfnWndProc = ::DefWindowProcA;
    _class.hInstance = instance;
    _class.lpszClassName = "wgl_extension_loader_class";
    _class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

    {
      auto result = ::RegisterClassA(&_class);
      if (!result && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        throw std::runtime_error("Failed to register WGL extension loader class.");
    }

    HWND handle = ::CreateWindowA(_class.lpszClassName, "Fake Window", WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0,
                                  0, 1, 1, NULL, NULL, instance, NULL);
    if (!handle)
      throw std::runtime_error("Failed to create dummy window for WGL extension loading.");

    HDC deviceContext = ::GetDC(handle);
    if (!deviceContext)
      throw std::runtime_error("Failed to get device context for dummy window.");

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
      if (pixelFormat == 0)
        throw std::runtime_error("Failed to choose pixel format for dummy window.");

      auto result = ::SetPixelFormat(deviceContext, pixelFormat, &pixelFormatDesc);
      if (!result)
        throw std::runtime_error("Failed to set pixel format for dummy window.");
    }

    HGLRC context = ::wglCreateContext(deviceContext);
    if (!context)
      throw std::runtime_error("Failed to create OpenGL context for dummy window.");

    {
      auto result = ::wglMakeCurrent(deviceContext, context);
      if (!result)
        throw std::runtime_error("Failed to make OpenGL context current for dummy window.");
    }

    {
      auto result = Krys::Gfx::OpenGL::InitialiseWGLHooks(deviceContext);
      if (!result)
        throw std::runtime_error("Failed to initialize WGL hooks.");

      result = Krys::Gfx::OpenGL::InitialiseGLHooks();
      if (!result)
        throw std::runtime_error("Failed to initialize OpenGL hooks.");
    }

    {
      auto result = ::wglMakeCurrent(NULL, NULL);
      if (!result)
        throw std::runtime_error("Failed to release OpenGL context for dummy window.");

      result = ::wglDeleteContext(context);
      if (!result)
        throw std::runtime_error("Failed to delete OpenGL context for dummy window.");

      result = ::ReleaseDC(handle, deviceContext);
      if (!result)
        throw std::runtime_error("Failed to release device context for dummy window.");

      result = ::DestroyWindow(handle);
      if (!result)
        throw std::runtime_error("Failed to destroy dummy window.");

      result = ::UnregisterClassA(_class.lpszClassName, instance);
      if (!result && ::GetLastError() != ERROR_CLASS_DOES_NOT_EXIST)
        throw std::runtime_error("Failed to unregister WGL extension loader class.");
    }
  }
}

#include "Krystal.Core/Detection.hpp"
#include "Krystal.Gfx.OpenGL/Hooks/gl.hpp"
#include "Krystal.Gfx.OpenGL/Hooks/wgl.hpp"
#include "Krystal.Gfx.OpenGL/OpenGLContext.hpp"
#include "Krystal.Gfx.OpenGL/Win32/GLContextPlatformImpl.hpp"

namespace Krys::Gfx::OpenGL
{
  OpenGLContext::GLContextPlatformImpl::GLContextPlatformImpl(NativeHandle nativeHandle)
      : _handle(nativeHandle.As<HWND>()), _deviceContext(nullptr), _renderingContext(nullptr)
  {
    const auto instance = ::GetModuleHandleA(NULL);
    if (!instance)
      throw std::runtime_error("Failed to get module handle for OpenGL context creation.");

    InitialiseExtensions(instance);

    _deviceContext = ::GetDC(_handle);
    if (!_deviceContext)
      throw std::runtime_error("Failed to get device context for OpenGL context creation.");

    SetupPixelFormat();
    SetupContext();
  }

  void OpenGLContext::GLContextPlatformImpl::SetupPixelFormat() const
  {
    using namespace Krys::Gfx::OpenGL;

    auto attributes = GetPixelFormatAttributes();
    int pixelFormat = 0;
    {
      UINT numFormats = 0;
      auto result =
        wglChoosePixelFormatARB(_deviceContext, attributes.data(), NULL, 1, &pixelFormat, &numFormats);

      if (!result || numFormats == 0 || pixelFormat == 0)
        throw std::runtime_error("Failed to choose pixel format for OpenGL context creation.");
    }

    PIXELFORMATDESCRIPTOR pixelFormatDesc {};
    {
      auto result =
        ::DescribePixelFormat(_deviceContext, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDesc);
      if (!result)
        throw std::runtime_error("Failed to describe pixel format for OpenGL context creation.");
    }

    {
      auto result = ::SetPixelFormat(_deviceContext, pixelFormat, &pixelFormatDesc);
      if (!result)
        throw std::runtime_error("Failed to set pixel format for OpenGL context creation.");
    }
  }

  List<int> OpenGLContext::GLContextPlatformImpl::GetPixelFormatAttributes() const noexcept
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
    bool multisampleCapable = true;
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

  void OpenGLContext::GLContextPlatformImpl::SetupContext()
  {
    auto attributes = GetContextAttributes();
    _renderingContext = wglCreateContextAttribsARB(_deviceContext, 0, attributes.data());
    if (!_renderingContext)
      throw std::runtime_error("Failed to create OpenGL rendering context.");

    auto result = wglMakeCurrent(_deviceContext, _renderingContext);
    if (!result)
      throw std::runtime_error("Failed to make OpenGL rendering context current.");
  }

  List<int> OpenGLContext::GLContextPlatformImpl::GetContextAttributes() const noexcept
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
//#ifdef KRYS_DEBUG
      ContextFlags |= WGL_CONTEXT_DEBUG_BIT_ARB;
//#endif
      attributes.push_back(ContextFlags);
    }

    attributes.push_back(0); // End of attributes

    return attributes;
  }

  void OpenGLContext::GLContextPlatformImpl::Present() const noexcept
  {
    assert(_deviceContext && _renderingContext);
    auto result = wglSwapBuffers(_deviceContext);
    assert(result);
  }
}