#pragma once

#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
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