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
  NO_DISCARD constexpr uint32 OrdinalCount<UI::MeasureMode>() noexcept
  {
    return 3;
  }
}