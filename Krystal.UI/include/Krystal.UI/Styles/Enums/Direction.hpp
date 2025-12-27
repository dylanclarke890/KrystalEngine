#pragma once

#include "Krystal.Lib/Enum.hpp"
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
  NO_DISCARD constexpr uint32 OrdinalCount<UI::Direction>() noexcept
  {
    return 3;
  }
}