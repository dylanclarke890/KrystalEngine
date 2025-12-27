#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class Unit : uint8
  {
    Undefined,
    Point,
    Percent,
    Auto,
    MaxContent,
    FitContent,
    Stretch,
  };
}

namespace Krys
{
  template <>
  NO_DISCARD constexpr uint32 OrdinalCount<UI::Unit>() noexcept
  {
    return 7;
  }
}