#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class Edge
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
  constexpr int32 OrdinalCount<UI::Edge>()
  {
    return 9;
  }
}