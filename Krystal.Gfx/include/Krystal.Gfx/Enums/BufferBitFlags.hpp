#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

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
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::Gfx::BufferBitFlags, 4u)
