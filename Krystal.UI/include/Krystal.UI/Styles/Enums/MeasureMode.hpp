#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::UI
{
  enum class MeasureMode : uint8
  {
    Undefined,
    Exactly,
    AtMost,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::UI::MeasureMode, 3u)
