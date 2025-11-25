#pragma once

#include "Krystal.Lib/Types.hpp"
#include "Krystal.Gfx/Vertex.hpp"

namespace Krys::Gfx
{
  enum class PrimitiveType : uint8
  {
    Points = 0,
    Lines,
    LineStrip,
    Triangles,
    TriangleStrip,
    TriangleFan,
  };

  enum class MeshType : uint8
  {
    Static,
    Dynamic,
  };

  struct MeshDesc
  {
    Span<const byte> VertexBuffer {};
    Span<const byte> IndexBuffer {};
    VertexBufferLayout Layout {};
    PrimitiveType Primitive {PrimitiveType::Triangles};
    MeshType Type {MeshType::Static};
  };
}