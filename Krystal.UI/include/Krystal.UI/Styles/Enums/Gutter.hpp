#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::UI
{
  enum class Gutter : uint8
  {
    Row,
    Column,
    All
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::UI::Gutter, 3u)
