#pragma once

#include "Krystal.Gfx/Colour.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys::Gfx
{
  struct PBRMaterialDesc
  {
    Colour AlbedoFactor {1.f, 1.f, 1.f, 1.f};
    float MetallicFactor {0.f};
    float RoughnessFactor {1.f};
    float AmbientOcclusionFactor {1.f};
    float NormalScale {1.f};
    Colour EmissiveFactor {0.f};

    TextureHandle AlbedoMap {};
    TextureHandle MetallicMap {};
    TextureHandle RoughnessMap {};
    TextureHandle AmbientOcclusionMap {};
    TextureHandle NormalMap {};
    TextureHandle EmissiveMap {};
  };

  class IMaterialSystem
  {
  public:
    virtual ~IMaterialSystem() = default;

    NO_DISCARD virtual MaterialHandle Create(const string &name, ShaderHandle shader,
                                             const PBRMaterialDesc &desc) noexcept = 0;

    virtual void Unload(MaterialHandle handle) noexcept = 0;
  };
}