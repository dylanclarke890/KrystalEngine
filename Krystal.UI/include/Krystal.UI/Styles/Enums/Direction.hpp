#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

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
  constexpr underlying_t<UI::Direction> OrdinalCount<UI::Direction> = 3;
}