#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  enum class BufferUsage : uint8
  {
    Static = 0,
    Dynamic,
    Stream,
  };

  enum class BufferType : uint8
  {
    Vertex = 0,
    Index,
    Uniform,
    Storage,
  };

  struct BufferDesc
  {
    BufferType Type {BufferType::Vertex};
    BufferUsage Usage {BufferUsage::Static};
    uint32 Size {0u};
    Span<const byte> InitialData {};
  };
}