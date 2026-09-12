#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::UI
{
  enum class BoxSizing : uint8
  {
    BorderBox,
    ContentBox,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::UI::BoxSizing, 2u)
