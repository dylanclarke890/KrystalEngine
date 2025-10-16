#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class Align : uint8
  {
    Auto,
    FlexStart,
    Center,
    FlexEnd,
    Stretch,
    Baseline,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
  };
}

namespace Krys
{
  template <>
  constexpr uint32 OrdinalCount<UI::Align>()
  {
    return 9;
  }
}