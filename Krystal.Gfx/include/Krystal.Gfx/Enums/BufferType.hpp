#pragma once

#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Gfx
{
  enum class BufferType : uint8
  {
    Vertex = 0,
    Index,
    Uniform,
    Storage,
  };
}