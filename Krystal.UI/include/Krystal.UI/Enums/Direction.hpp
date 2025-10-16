#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class Direction : uint8
  {
    Inherit,
    LTR,
    RTL
  };
}

namespace Krys
{
  template <>
  constexpr uint32 OrdinalCount<UI::Direction>()
  {
    return 3;
  }
}