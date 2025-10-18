#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  class IBufferRegistry
  {
    NO_COPY_MOVE(IBufferRegistry)

  protected:
    IBufferRegistry() noexcept = default;

  public:
    virtual ~IBufferRegistry() = default;

    virtual void Startup() noexcept = 0;

    virtual void Shutdown() noexcept = 0;

    NO_DISCARD virtual BufferHandle CreateUniformBuffer(uint32 size) = 0;
    NO_DISCARD virtual BufferHandle CreateUniformBuffer(Span<const byte> data) = 0;

    NO_DISCARD virtual BufferHandle CreateIndexBuffer(uint32 size) = 0;
    NO_DISCARD virtual BufferHandle CreateIndexBuffer(Span<const byte> data) = 0;

    NO_DISCARD virtual BufferHandle CreateVertexBuffer(uint32 size) = 0;
    NO_DISCARD virtual BufferHandle CreateVertexBuffer(Span<const byte> data) = 0;

    virtual void Destroy(BufferHandle handle) = 0;
  };
}