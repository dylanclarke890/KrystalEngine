#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::UI
{
  enum class Justify : uint8
  {
    FlexStart,
    Center,
    FlexEnd,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::UI::Justify, 6u)
