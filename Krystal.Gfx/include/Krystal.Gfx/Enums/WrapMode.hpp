#pragma once

#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  enum class WrapMode : uint8
  {
    Repeat = 0,
    ClampToEdge,
    ClampToBorder,
  };
}