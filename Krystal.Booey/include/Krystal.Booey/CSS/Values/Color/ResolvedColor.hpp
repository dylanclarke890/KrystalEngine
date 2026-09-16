#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct ResolvedColor
  {
    boo::Color value;

    bool operator==(const ResolvedColor &) const = default;
  };

  KRYS_NODISCARD inline boo::Color CreateColor(const ResolvedColor &unresolved,
                                               PlatformColorResolutionState &) noexcept
  {
    return unresolved.value;
  }

  KRYS_NODISCARD constexpr bool containsColorSchemeDependentColor(const ResolvedColor &) noexcept
  {
    return false;
  }

  KRYS_NODISCARD constexpr bool containsCurrentColor(const ResolvedColor &) noexcept
  {
    return false;
  }
}