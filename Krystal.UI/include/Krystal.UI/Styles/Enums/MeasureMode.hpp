#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class MeasureMode : uint8
  {
    Undefined,
    Exactly,
    AtMost,
  };
}

namespace Krys
{
  template <>
  constexpr UnderlyingType<UI::MeasureMode> OrdinalCount<UI::MeasureMode> = 3;
}