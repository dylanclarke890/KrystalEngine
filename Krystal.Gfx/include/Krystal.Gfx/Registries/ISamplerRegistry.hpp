#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Sampler.hpp"

namespace krys::Gfx
{
  class ISamplerRegistry
  {
    KRYS_NON_COPY_MOVABLE(ISamplerRegistry);

  protected:
    ISamplerRegistry() noexcept = default;

  public:
    virtual ~ISamplerRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    KRYS_NODISCARD virtual SamplerHandle Create(const SamplerDesc &desc) noexcept = 0;

    virtual bool Unload(SamplerHandle handle) noexcept = 0;
  };
}