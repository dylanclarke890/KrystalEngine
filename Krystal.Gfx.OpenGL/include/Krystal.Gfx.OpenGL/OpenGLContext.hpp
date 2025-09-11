#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.IO/Image.hpp"

namespace Krys::Gfx::OpenGL
{
  class OpenGLContext final : public IContext
  {
    NO_COPY_MOVE(OpenGLContext)

    NativeHandle _windowHandle;
    uint32 _width;
    uint32 _height;

    class GLContextPlatformImpl;
    Unique<GLContextPlatformImpl> _platformImpl;

  public:
    OpenGLContext(NativeHandle windowHandle, uint32 width, uint32 height);

    ~OpenGLContext() noexcept override = default;

    void Setup() noexcept override;

    void Render(ICamera &camera) noexcept override;

    void Present() noexcept override;

    virtual void Resize(uint32 width, uint32 height) noexcept override;
  };
}
