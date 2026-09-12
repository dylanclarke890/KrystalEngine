#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::Gfx
{
  enum class WrapMode : uint8
  {
    Repeat = 0,
    ClampToEdge,
    ClampToBorder,
  };
}