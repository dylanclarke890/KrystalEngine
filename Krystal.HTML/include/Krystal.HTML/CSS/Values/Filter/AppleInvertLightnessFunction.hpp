#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // Non-standard addition.
  struct AppleInvertLightness
  {
    constexpr bool operator==(const AppleInvertLightness &) const = default;
  };

  using AppleInvertLightnessFunction =
    FunctionNotation<CSSValueId::AppleInvertLightness, AppleInvertLightness>;
}

DEFINE_EMPTY_LIKE_CONFORMANCE(Krys::HTML::AppleInvertLightness)
