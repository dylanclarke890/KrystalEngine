#pragma once

#include "Krystal.Gfx.OpenGL/Context.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"

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