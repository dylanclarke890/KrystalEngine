#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::UI
{
  enum class Errata : uint8
  {
    None,
    StretchFlexBasis = 1 << 0,
    AbsolutePositionWithoutInsetsExcludesPadding = 1 << 1,
    AbsolutePercentAgainstInnerSize = 1 << 2,
    All = StretchFlexBasis | AbsolutePositionWithoutInsetsExcludesPadding | AbsolutePercentAgainstInnerSize,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::UI::Errata, 4u)
