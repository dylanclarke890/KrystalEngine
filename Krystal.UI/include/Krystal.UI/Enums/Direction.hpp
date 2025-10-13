#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class Direction
  {
    Inherit,
    LTR,
    RTL
  };
}

namespace Krys
{
  template <>
  constexpr int32 OrdinalCount<UI::Direction>()
  {
    return 3;
  }
}