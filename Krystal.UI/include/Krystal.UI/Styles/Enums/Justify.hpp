#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
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
  NO_DISCARD constexpr uint32 OrdinalCount<UI::Justify>() noexcept
  {
    return 6;
  }
}