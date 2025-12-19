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

  template <typename T>
  concept HasVertexLayout = requires() { T::Layout(); };

  template <typename T>
  concept IsMeshIndex = std::is_integral_v<T>;

  template <HasVertexLayout TVertex, IsMeshIndex TIndex>
  class MeshDataWriter
  {
    using Vertex = TVertex;
    using Index = TIndex;

  private:
    MeshData &_data;

  public:
    constexpr MeshDataWriter(MeshData &data) noexcept : _data(data)
    {
    }

    constexpr size_t TotalVertices()
    {
      return _data.Vertices.size() / sizeof(Vertex);
    }

    constexpr void ReserveVertices(size_t count)
    {
      _data.Vertices.reserve(count * sizeof(Vertex));
    }

    constexpr void ResizeVertices(size_t count)
    {
      _data.Vertices.resize(count * sizeof(Vertex));
    }

    constexpr Vertex *Vertices()
    {
      return reinterpret_cast<Vertex *>(_data.Vertices.data());
    }

    constexpr size_t TotalIndices()
    {
      return _data.Indices.size() / sizeof(Index);
    }

    constexpr void ReserveIndices(size_t count)
    {
      _data.Indices.reserve(count * sizeof(Index));
    }

    constexpr void ResizeIndices(size_t count)
    {
      _data.Indices.resize(count * sizeof(Index));
    }

    constexpr Index *Indices()
    {
      return reinterpret_cast<Index *>(_data.Indices.data());
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