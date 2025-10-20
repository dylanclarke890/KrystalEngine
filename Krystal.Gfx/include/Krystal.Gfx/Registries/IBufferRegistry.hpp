#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Buffer.hpp"
#include "Krystal.Lib/Macros.hpp"

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

    NO_DISCARD virtual BufferHandle Create(const BufferDesc &desc) = 0;

    virtual bool Destroy(BufferHandle handle) = 0;
  };
}