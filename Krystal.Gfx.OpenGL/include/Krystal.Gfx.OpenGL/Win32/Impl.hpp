#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Gfx.OpenGL/Context.hpp"
#include "Krystal.Gfx.OpenGL/Hooks/wgl.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif

namespace Krys::Gfx
{
  class OpenGLContext::Impl
  {
    NO_COPY_MOVE(Impl)

    HWND _handle;
    HDC _deviceContext;
    HGLRC _renderingContext;

  public:
    explicit Impl(NativeHandle nativeHandle) noexcept
        : _handle(nativeHandle.As<HWND>()), _deviceContext(nullptr), _renderingContext(nullptr)
    {
    }

    void Initialise() noexcept;

    void Present() const noexcept;

  private:
    void SetupPixelFormat() const noexcept;
    List<int> GetPixelFormatAttributes() const noexcept;

    void SetupContext() noexcept;
    List<int> GetContextAttributes() const noexcept;
  };
}