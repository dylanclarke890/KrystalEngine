#pragma once

#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Span.hpp"
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

  struct MeshData
  {
    List<byte> Vertices {};
    List<byte> Indices {};
  };

  struct MeshDesc
  {
    Span<const byte> Vertices {};
    Span<const byte> Indices {};
    VertexBufferLayout Layout {};
    PrimitiveType Primitive {PrimitiveType::Triangles};
    MeshType Type {MeshType::Static};
  };
}