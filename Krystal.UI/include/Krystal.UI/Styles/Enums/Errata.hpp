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

namespace Krys
{
  template <>
  constexpr underlying_t<UI::Errata> OrdinalCount<UI::Errata> = 4;

  template <>
  inline constexpr bool EnableEnumFlags<UI::Errata> = true;
}