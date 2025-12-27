#pragma once

#include "Krystal.Gfx/Enums/BufferType.hpp"
#include "Krystal.Gfx/Enums/BufferUsage.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  struct BufferDesc
  {
    BufferType Type {BufferType::Vertex};
    BufferUsage Usage {BufferUsage::Static};
    uint32 Size {0u};
    Span<const byte> InitialData {};
  };
}