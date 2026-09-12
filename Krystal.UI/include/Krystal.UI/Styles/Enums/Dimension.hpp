#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::UI
{
  enum class Dimension : uint8
  {
    Width,
    Height
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::UI::Dimension, 2u)
