#pragma once

#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Gfx
{
  enum class WrapMode : uint8
  {
    Repeat = 0,
    ClampToEdge,
    ClampToBorder,
  };
}