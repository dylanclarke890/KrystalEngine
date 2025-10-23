#pragma once

#include "Krystal.Lib/Types.hpp"

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
    MeshType Type {MeshType::Static};
    PrimitiveType Primitive {PrimitiveType::Triangles};
    VertexBufferLayout Layout {};
    Span<const byte> VertexBuffer {};
    Span<const byte> IndexBuffer {};
  };
}