#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class TextAlign : uint8
  {
    Left,
    Center,
    Right,
    Justify
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::UI::TextAlign, 4u)
