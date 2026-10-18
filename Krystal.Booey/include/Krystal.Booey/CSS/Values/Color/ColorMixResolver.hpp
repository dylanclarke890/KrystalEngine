#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  struct ColorMixResolver
  {
    struct Component
    {
      using PercentageType = style::Percentage<Range {0, 100}>;

      boo::Color Color;
      Maybe<PercentageType> Percentage;
    };

    krys::ColourInterpolationMethod ColourInterpolationMethod;
    Component MixComponents1;
    Component MixComponents2;
  };

  KRYS_NODISCARD boo::Color Mix(const ColorMixResolver &colorMix) noexcept;
}
