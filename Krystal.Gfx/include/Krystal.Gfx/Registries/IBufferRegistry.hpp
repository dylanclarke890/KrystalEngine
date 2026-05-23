#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Buffer.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"

namespace Krys::Gfx
{
  class IBufferRegistry : NonCopyMovable<IBufferRegistry>
  {
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