#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  // Non-standard addition.
  struct AppleInvertLightness
  {
    constexpr bool operator==(const AppleInvertLightness &) const = default;
  };

  using AppleInvertLightnessFunction = FunctionNotation<ValueId::AppleInvertLightness, AppleInvertLightness>;
}

DEFINE_EMPTY_LIKE_CONFORMANCE(krys::boo::css::AppleInvertLightness)
