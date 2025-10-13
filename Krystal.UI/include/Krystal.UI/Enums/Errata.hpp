#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class Errata
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