#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Gfx.OpenGL/Hooks/wgl.hpp"
#include "Krystal.Gfx.OpenGL/OpenGLContext.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #undef CreateWindow
  #undef LoadImage
#endif

#pragma comment(lib, "opengl32.lib")

namespace Krys::Gfx::OpenGL
{
  class OpenGLContext::GLContextPlatformImpl
  {
    NO_COPY_MOVE(GLContextPlatformImpl)

    HWND _handle;
    HDC _deviceContext;
    HGLRC _renderingContext;

  public:
    explicit GLContextPlatformImpl(NativeHandle nativeHandle);

    void Present() const noexcept;

  private:
    void SetupPixelFormat() const;
    List<int> GetPixelFormatAttributes() const noexcept;

    void SetupContext();
    List<int> GetContextAttributes() const noexcept;
  };
}