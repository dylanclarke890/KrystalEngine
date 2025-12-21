#pragma once

#include "Krystal.Lib/Types.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Gfx
{
  enum class BufferBitFlags : uint8
  {
    None = 0,
    Colour = 1 << 0,
    Depth = 1 << 1,
    Stencil = 1 << 2,
    All = Colour | Depth | Stencil,
  };

  ENUM_BITWISE_OPERATORS(BufferBitFlags)
}