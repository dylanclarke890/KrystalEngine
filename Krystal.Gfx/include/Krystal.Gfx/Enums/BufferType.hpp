#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::Gfx
{
  enum class BufferType : uint8
  {
    Vertex = 0,
    Index,
    Uniform,
    Storage,
  };
}