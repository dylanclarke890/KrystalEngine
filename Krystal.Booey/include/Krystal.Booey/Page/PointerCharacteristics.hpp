#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief What a pointing device can do: point coarsely (a touch screen), finely (a mouse or pen), both
  /// (several devices together) or neither (there's none).
  enum class PointerCharacteristics : uint8
  {
    None = 0,
    Coarse = 1 << 0,
    Fine = 1 << 1,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::PointerCharacteristics, 2uz);
