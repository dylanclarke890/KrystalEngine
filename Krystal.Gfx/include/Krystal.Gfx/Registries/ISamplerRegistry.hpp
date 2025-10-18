#pragma once

#include "Krystal.Gfx/Common.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Gfx
{
  struct SamplerDesc
  {
    FilterMode MinFilter {FilterMode::Linear};
    FilterMode MagFilter {FilterMode::Linear};
    WrapMode WrapS {WrapMode::Repeat};
    WrapMode WrapT {WrapMode::Repeat};
    WrapMode WrapR {WrapMode::Repeat};
    float AnisotropicLevel {1.0f};
  };

  class ISamplerRegistry
  {
    NO_COPY_MOVE(ISamplerRegistry)

  protected:
    ISamplerRegistry() noexcept = default;

  public:
    virtual ~ISamplerRegistry() = default;

    virtual void Startup() noexcept = 0;

    virtual void Shutdown() noexcept = 0;

    NO_DISCARD virtual SamplerHandle Create(const SamplerDesc &desc) noexcept = 0;

    virtual void Unload(SamplerHandle handle) noexcept = 0;
  };
}