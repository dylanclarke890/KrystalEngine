#pragma once

#include "Krystal.Gfx.Lib/ResourceManager.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Buffer.hpp"
#include "Krystal.Gfx/Registries/IBufferRegistry.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Gfx::OpenGL
{
  class BufferRegistry : public IBufferRegistry
  {
    NO_COPY_MOVE(BufferRegistry)

    using BufferManager = ResourceManager<Buffer, BufferHandle>;

  private:
    BufferManager _buffers;

  public:
    BufferRegistry() noexcept = default;

    virtual ~BufferRegistry() noexcept = default;

    void Startup() override
    {
    }

    void Shutdown() noexcept override
    {
    }

    NO_DISCARD virtual BufferHandle Create(const BufferDesc &desc) override
    {
      GLenum bufferType = MapBufferType(desc.Type);
      GLenum bufferUsage = MapBufferUsage(desc.Usage);
      Buffer buffer {bufferType, bufferUsage, desc.Size, desc.InitialData};
      return _buffers.Add(std::move(buffer));
    }

    bool Destroy(BufferHandle handle) override
    {
      return _buffers.Remove(handle);
    }

    NO_DISCARD Buffer &Get(BufferHandle handle)
    {
      return _buffers.Get(handle);
    }

    NO_DISCARD Buffer *TryGet(BufferHandle handle) noexcept
    {
      return _buffers.TryGet(handle);
    }
  };
}