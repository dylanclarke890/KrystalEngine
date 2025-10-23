#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI::Styles
{
  enum class Justify : uint8
  {
    FlexStart,
    Center,
    FlexEnd,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
  };
}

namespace Krys
{
  template <>
  constexpr uint32 OrdinalCount<UI::Styles::Justify>()
  {
    return 6;
  }
}