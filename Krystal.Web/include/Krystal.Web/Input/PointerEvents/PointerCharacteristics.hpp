#pragma once

#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::web
{
  /// @brief What a pointing device can do: point coarsely (a touch screen), finely (a mouse or pen), both
  /// (several devices together) or neither (there's none).
  enum class PointerCharacteristic : uint8
  {
    Coarse,
    Fine,
  };

  consteval PointerCharacteristic MaxEnumValue(PointerCharacteristic) noexcept
  {
    return PointerCharacteristic::Fine;
  }

  using PointerCharacteristics = EnumSet<PointerCharacteristic>;
}
