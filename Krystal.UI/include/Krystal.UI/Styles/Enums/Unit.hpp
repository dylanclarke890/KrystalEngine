#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class Unit : uint8
  {
    Undefined,
    Point,
    Percent,
    Auto,
    MaxContent,
    FitContent,
    Stretch,
  };
}

namespace Krys
{
  template <>
  constexpr UnderlyingType<UI::Unit> OrdinalCount<UI::Unit> = 7;
}