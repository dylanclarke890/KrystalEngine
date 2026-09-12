#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::UI
{
  enum class TextAlign : uint8
  {
    Left,
    Center,
    Right,
    Justify
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::UI::TextAlign, 4u)
