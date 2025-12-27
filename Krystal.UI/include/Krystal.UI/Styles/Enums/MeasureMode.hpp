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
  NO_DISCARD constexpr uint32 OrdinalCount<UI::MeasureMode>() noexcept
  {
    return 3;
  }
}