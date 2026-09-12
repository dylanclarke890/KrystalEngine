#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Gfx/Enums/BufferType.hpp"
#include "Krystal.Gfx/Enums/BufferUsage.hpp"

namespace krys::Gfx
{
  struct BufferDesc
  {
    BufferType Type {BufferType::Vertex};
    BufferUsage Usage {BufferUsage::Static};
    uint32 Size {0u};
    Span<const byte> InitialData {};
  };
}