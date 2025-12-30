#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class Edge : uint8
  {
    Left,
    Top,
    Right,
    Bottom,
    Start,
    End,
    Horizontal,
    Vertical,
    All
  };
}

namespace Krys
{
  template <>
  constexpr UnderlyingType<UI::Edge> OrdinalCount<UI::Edge> = 9;
}