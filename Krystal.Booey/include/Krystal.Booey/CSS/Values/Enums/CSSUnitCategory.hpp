#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class CSSUnitCategory : uint8
  {
    Number,
    Percent,
    AbsoluteLength,
    FontRelativeLength,
    ViewportPercentageLength,
    Angle,
    Time,
    Frequency,
    Resolution,
    Flex,
    Other
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::CSSUnitCategory, 10uz);
