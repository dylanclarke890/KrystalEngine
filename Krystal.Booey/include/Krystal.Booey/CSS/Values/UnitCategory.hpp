#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class UnitCategory : uint8
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::UnitCategory, 11uz);
