#pragma once

#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Sampler.hpp"
#include "Krystal.Gfx.OpenGL/Mappers/Enums/WrapMode.hpp"
#include "Krystal.Gfx.OpenGL/Mappers/Enums/FilterMode.hpp"
#include "Krystal.Gfx/Registries/ISamplerRegistry.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Hash.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Map.hpp"

namespace Krys::Gfx::OpenGL
{
  class SamplerRegistry final : public ISamplerRegistry
  {
    NO_COPY_MOVE(SamplerRegistry)

    using SamplerManager = ResourceManager<Sampler, SamplerHandle>;
    using SamplerCache = ResourceHandleCache<SamplerDesc, SamplerHandle>;

  private:
    SamplerManager _samplers;
    SamplerCache _cache;

  public:
    SamplerRegistry() = default;

    ~SamplerRegistry() noexcept override = default;

    void Startup() override
    {
    }

    void Shutdown() noexcept override
    {
    }

    KRYS_NODISCARD SamplerHandle Create(const SamplerDesc &desc) noexcept override
    {
      if (auto cached = _cache.Get(desc); cached.IsValid())
      {
        return cached;
      }

      GLenum mag = MapFilterMode(desc.MagFilter);
      assert(mag != GL_NEAREST_MIPMAP_NEAREST && mag != GL_LINEAR_MIPMAP_NEAREST
             && mag != GL_NEAREST_MIPMAP_LINEAR && mag != GL_LINEAR_MIPMAP_LINEAR
             && "Mag filter cannot be a mipmap filter.");

      GLenum min = MapFilterMode(desc.MinFilter);

      GLenum wrapS = MapWrapMode(desc.WrapS);
      GLenum wrapT = MapWrapMode(desc.WrapT);
      GLenum wrapR = MapWrapMode(desc.WrapR);

      Sampler sampler {min, mag, wrapS, wrapT, wrapR, desc.AnisotropicLevel};
      return Add(std::move(sampler), desc);
    }

    bool Unload(SamplerHandle handle) noexcept override
    {
      assert(handle.IsValid() && "Invalid handle.");
      assert(_samplers.TryGet(handle) != nullptr && "Sampler not found in resource manager.");

      if (_cache.Remove(handle))
      {
        return _samplers.Remove(handle);
      }
      return false;
    }

    KRYS_NODISCARD Sampler &Get(SamplerHandle handle)
    {
      return _samplers.Get(handle);
    }

  private:
    KRYS_NODISCARD SamplerHandle Add(Sampler &&sampler, const SamplerDesc &cacheKey) noexcept
    {
      auto handle = _samplers.Add(std::move(sampler));
      _cache.Add(cacheKey, handle);
      return handle;
    }
  };
}