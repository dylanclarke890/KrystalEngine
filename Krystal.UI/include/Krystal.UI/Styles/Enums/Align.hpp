#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::UI
{
  enum class Align : uint8
  {
    Auto,
    FlexStart,
    Center,
    FlexEnd,
    Stretch,
    Baseline,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::UI::Align, 9u)
