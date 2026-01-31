#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class BoxSizing : uint8
  {
    BorderBox,
    ContentBox,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::UI::BoxSizing, 2u)
