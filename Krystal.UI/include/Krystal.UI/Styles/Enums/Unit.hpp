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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::UI::Unit, 7u)
