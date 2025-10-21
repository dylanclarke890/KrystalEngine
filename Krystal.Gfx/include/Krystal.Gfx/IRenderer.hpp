#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  class IRenderer
  {
    NO_COPY_MOVE(IRenderer)

  public:
    IRenderer() noexcept = default;

    virtual ~IRenderer() = default;
  };
}