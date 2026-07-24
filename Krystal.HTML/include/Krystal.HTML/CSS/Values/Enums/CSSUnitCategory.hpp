#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSUnitCategory, 10uz);
