#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::UI
{
  enum class PositionType : uint8
  {
    Static,
    Relative,
    Absolute,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::UI::PositionType, 3u)
