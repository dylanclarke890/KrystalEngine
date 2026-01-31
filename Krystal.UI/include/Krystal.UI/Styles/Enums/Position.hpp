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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::UI::PositionType, 3u)
