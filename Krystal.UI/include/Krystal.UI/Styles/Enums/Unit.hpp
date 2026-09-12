#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::UI
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::UI::Unit, 7u)
