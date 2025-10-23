#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI::Styles
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
  constexpr uint32 OrdinalCount<UI::Styles::Edge>()
  {
    return 9;
  }
}