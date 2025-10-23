#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI::Styles
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
  constexpr uint32 OrdinalCount<UI::Styles::MeasureMode>()
  {
    return 3;
  }
}