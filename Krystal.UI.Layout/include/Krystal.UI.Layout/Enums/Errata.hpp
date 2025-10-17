#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI::Layout
{
  enum class Errata : uint8
  {
    None,
    StretchFlexBasis,
    AbsolutePositionWithoutInsetsExcludesPadding,
    AbsolutePercentAgainstInnerSize,
    All,
    Classic,
  };

  ENUM_BITWISE_OPERATORS(Errata)
}