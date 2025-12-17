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

    template <typename TVertex>
    constexpr size_t TotalVertices()
    {
      return Vertices.size() / sizeof(TVertex);
    }

    template <typename TIndex = uint32>
    constexpr size_t TotalIndices()
    {
      return Indices.size() / sizeof(TIndex);
    }
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