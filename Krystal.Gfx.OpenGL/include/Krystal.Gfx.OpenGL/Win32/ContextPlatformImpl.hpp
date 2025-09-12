#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Gfx.OpenGL/Hooks/wgl.hpp"
#include "Krystal.Gfx.OpenGL/Context.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #undef CreateWindow
  #undef LoadImage
#endif

#pragma comment(lib, "opengl32.lib")

namespace Krys::Gfx::OpenGL
{
  class Context::ContextPlatformImpl
  {
    NO_COPY_MOVE(ContextPlatformImpl)

    HWND _handle;
    HDC _deviceContext;
    HGLRC _renderingContext;

  public:
    explicit ContextPlatformImpl(NativeHandle nativeHandle);

    void Present() const noexcept;

  private:
    void SetupPixelFormat() const;
    List<int> GetPixelFormatAttributes() const noexcept;

    void SetupContext();
    List<int> GetContextAttributes() const noexcept;
  };
}