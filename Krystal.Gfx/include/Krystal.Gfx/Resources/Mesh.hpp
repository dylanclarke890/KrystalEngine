#pragma once

#include "Krystal.Gfx/Enums/MeshType.hpp"
#include "Krystal.Gfx/Enums/PrimitiveType.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
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

    NO_DISCARD constexpr size_t TotalVertices() const noexcept
    {
      return _data.Vertices.size() / sizeof(Vertex);
    }

    constexpr void ReserveVertices(size_t count) noexcept
    {
      _data.Vertices.reserve(count * sizeof(Vertex));
    }

    constexpr void ResizeVertices(size_t count) noexcept
    {
      _data.Vertices.resize(count * sizeof(Vertex));
    }

    NO_DISCARD constexpr Vertex *Vertices() noexcept
    {
      return reinterpret_cast<Vertex *>(_data.Vertices.data());
    }

    NO_DISCARD constexpr size_t TotalIndices() const noexcept
    {
      return _data.Indices.size() / sizeof(Index);
    }

    constexpr void ReserveIndices(size_t count) noexcept
    {
      _data.Indices.reserve(count * sizeof(Index));
    }

    constexpr void ResizeIndices(size_t count) noexcept
    {
      _data.Indices.resize(count * sizeof(Index));
    }

    NO_DISCARD constexpr Index *Indices() noexcept
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