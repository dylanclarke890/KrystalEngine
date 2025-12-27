#pragma once

#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Gfx
{
  enum class BufferUsage : uint8
  {
    Static = 0,
    Dynamic,
    Stream,
  };
}