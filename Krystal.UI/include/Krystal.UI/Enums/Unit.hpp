#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class Unit
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
  constexpr int32 OrdinalCount<UI::Unit>()
  {
    return 7;
  }
}