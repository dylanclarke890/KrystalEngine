#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Attributes.hpp"

namespace Krys::Gfx
{
  enum class FilterMode : uint8
  {
    Nearest = 0,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear,
  };

  enum class WrapMode : uint8
  {
    Repeat = 0,
    ClampToEdge,
    ClampToBorder,
  };

  struct SamplerDesc 
  {
    FilterMode MinFilter {FilterMode::Linear};
    FilterMode MagFilter {FilterMode::Linear};
    WrapMode WrapS {WrapMode::Repeat};
    WrapMode WrapT {WrapMode::Repeat};
    WrapMode WrapR {WrapMode::Repeat};
    float AnisotropicLevel {1.0f};
  };

  class ISamplerSystem
  {
  public:
    virtual ~ISamplerSystem() = default;

    NO_DISCARD virtual SamplerHandle Create(const SamplerDesc &desc) noexcept = 0;

    virtual void Unload(SamplerHandle handle) noexcept = 0;
  };
}