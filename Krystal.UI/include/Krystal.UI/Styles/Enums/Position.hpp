#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class PositionType : uint8
  {
    Static,
    Relative,
    Absolute,
  };
}

namespace Krys
{
  template <>
  constexpr UnderlyingType<UI::PositionType> OrdinalCount<UI::PositionType> = 3;
}