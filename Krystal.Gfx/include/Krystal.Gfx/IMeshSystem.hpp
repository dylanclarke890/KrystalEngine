#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/VertexBufferLayout.hpp"
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

  struct MeshHandle : Handle<MeshHandle>
  {
  };

  class IMeshSystem
  {
  public:
    virtual ~IMeshSystem() = default;

    NO_DISCARD virtual MeshHandle Create(const Span<const byte> &vertexBuffer,
                                         const VertexBufferLayout &layout,
                                         PrimitiveType type = PrimitiveType::Triangles) noexcept = 0;

    NO_DISCARD virtual MeshHandle Create(const Span<const byte> &vertexBuffer,
                                         const Span<const byte> &indexBuffer,
                                         const VertexBufferLayout &layout,
                                         PrimitiveType type = PrimitiveType::Triangles) noexcept = 0;

    virtual void Unload(MeshHandle handle) noexcept = 0;
  };
}