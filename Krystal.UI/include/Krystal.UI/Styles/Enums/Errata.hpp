#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
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

  KRYS_ENUM_FLAG_OPERATORS(Errata)
}