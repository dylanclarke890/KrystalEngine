#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

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
  constexpr int32 OrdinalCount<UI::MeasureMode>()
  {
    return 3;
  }
}