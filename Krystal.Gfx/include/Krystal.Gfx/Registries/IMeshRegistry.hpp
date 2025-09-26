#pragma once

#include "Krystal.Gfx/Common.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/VertexBufferLayout.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  class IMeshRegistry
  {
  public:
    virtual ~IMeshRegistry() = default;

    virtual void Startup() noexcept = 0;

    virtual void Shutdown() noexcept = 0;

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