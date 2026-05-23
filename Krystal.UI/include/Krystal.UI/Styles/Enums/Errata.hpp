#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
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

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::UI::Errata, 4u)
