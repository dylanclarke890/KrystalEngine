#pragma once

#include "Krystal.Booey/Core/Color/Color.hpp"
#include "Krystal.Booey/Core/Color/ColorTypes.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct HexColor
  {
    SRGBA<uint8> value;

    bool operator==(const HexColor &) const = default;
  };

  KRYS_NODISCARD inline boo::Color CreateColor(const HexColor &unresolved,
                                               PlatformColorResolutionState &) noexcept
  {
    return boo::Color {unresolved.value};
  }

  KRYS_NODISCARD constexpr bool ContainsCurrentColor(const HexColor &) noexcept
  {
    return false;
  }

  KRYS_NODISCARD constexpr bool ContainsColorSchemeDependentColor(const HexColor &) noexcept
  {
    return false;
  }
}