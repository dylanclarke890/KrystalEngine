#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Buffer.hpp"

namespace krys::Gfx
{
  class IBufferRegistry
  {
    KRYS_NON_COPY_MOVABLE(IBufferRegistry);

  protected:
    IBufferRegistry() noexcept = default;

  public:
    virtual ~IBufferRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    KRYS_NODISCARD virtual BufferHandle Create(const BufferDesc &desc) = 0;

    virtual bool Destroy(BufferHandle handle) = 0;
  };
}