#pragma once

#include "Krystal.Gfx.Lib/ResourceManager.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Buffer.hpp"
#include "Krystal.Gfx/Registries/IBufferRegistry.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

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

    virtual ~BufferRegistry() = default;

    void Startup() noexcept override
    {
    }

    void Shutdown() noexcept override
    {
    }

    virtual BufferHandle CreateUniformBuffer(uint32 size) override
    {
      return CreateBuffer(GL_UNIFORM_BUFFER, size);
    }

    virtual BufferHandle CreateUniformBuffer(Span<const byte> data) override
    {
      return CreateBuffer(GL_UNIFORM_BUFFER, data);
    }

    virtual BufferHandle CreateIndexBuffer(uint32 size) override
    {
      return CreateBuffer(GL_ELEMENT_ARRAY_BUFFER, size);
    }

    virtual BufferHandle CreateIndexBuffer(Span<const byte> data) override
    {
      return CreateBuffer(GL_ELEMENT_ARRAY_BUFFER, data);
    }

    virtual BufferHandle CreateVertexBuffer(uint32 size) override
    {
      return CreateBuffer(GL_ARRAY_BUFFER, size);
    }

    virtual BufferHandle CreateVertexBuffer(Span<const byte> data) override
    {
      return CreateBuffer(GL_ARRAY_BUFFER, data);
    }

    BufferHandle CreateBuffer(GLenum bufferType, const BufferData &data) noexcept
    {
      return _buffers.Add(Buffer(bufferType, data));
    }

    BufferHandle CreateBuffer(GLenum bufferType, uint32 size) noexcept
    {
      return _buffers.Add(Buffer(bufferType, size));
    }

    void Destroy(BufferHandle handle) override
    {
      _buffers.Remove(handle);
    }

    Buffer &Get(BufferHandle handle)
    {
      return _buffers.Get(handle);
    }
  };
}