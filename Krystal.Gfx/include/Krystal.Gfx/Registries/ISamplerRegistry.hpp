#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Sampler.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Gfx
{
  class ISamplerRegistry
  {
    NO_COPY_MOVE(ISamplerRegistry)

  protected:
    ISamplerRegistry() noexcept = default;

  public:
    virtual ~ISamplerRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    NO_DISCARD virtual SamplerHandle Create(const SamplerDesc &desc) noexcept = 0;

    virtual bool Unload(SamplerHandle handle) noexcept = 0;
  };
}