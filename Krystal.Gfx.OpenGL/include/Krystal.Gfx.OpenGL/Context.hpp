#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Gfx/IContext.hpp"

namespace Krys::Gfx
{
  class OpenGLContext final : public IContext
  {
    NO_COPY_MOVE(OpenGLContext)
    class Impl;

    NativeHandle _windowHandle;
    Unique<Impl> _impl;

  public:
    OpenGLContext(NativeHandle windowHandle) noexcept;

    ~OpenGLContext() noexcept override = default;

    void Initialise() noexcept override;

    void Present() noexcept override;
  };
}
