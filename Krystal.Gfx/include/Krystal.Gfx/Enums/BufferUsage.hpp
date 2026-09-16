#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::Gfx
{
  enum class BufferUsage : uint8
  {
    Static = 0,
    Dynamic,
    Stream,
  };
}