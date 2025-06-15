#pragma once

#include "Krystal.Core/Core.hpp"

namespace Krys::Gfx
{
  class IContext;

  Expected<Unique<IContext>> CreateContext(NativeHandle windowHandle) noexcept;

  class IContext
  {
    NO_COPY_MOVE(IContext)

  protected:
    IContext() noexcept = default;

  public:
    virtual ~IContext() noexcept = default;

    virtual void Present() noexcept = 0;
  };
}