#pragma once

#include "Krystal.Gfx.Lib/ResourceManager.hpp"
#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Sampler.hpp"
#include "Krystal.Gfx/Registries/ISamplerRegistry.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/HashUtils.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"

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

    NO_DISCARD SamplerHandle Create(const SamplerDesc &desc) noexcept override
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

    NO_DISCARD Sampler &Get(SamplerHandle handle)
    {
      return _samplers.Get(handle);
    }

  private:
    NO_DISCARD SamplerHandle Add(Sampler &&sampler, const SamplerDesc &cacheKey) noexcept
    {
      auto handle = _samplers.Add(std::move(sampler));
      _cache.Add(cacheKey, handle);
      return handle;
    }

    NO_DISCARD static GLenum MapFilterMode(FilterMode mode) noexcept
    {
      switch (mode)
      {
        case FilterMode::Nearest:              return GL_NEAREST;
        case FilterMode::Linear:               return GL_LINEAR;
        case FilterMode::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case FilterMode::LinearMipmapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
        case FilterMode::NearestMipmapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
        case FilterMode::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
        default:                               return GL_LINEAR;
      }
    }

    NO_DISCARD static GLenum MapWrapMode(WrapMode mode) noexcept
    {
      switch (mode)
      {
        case WrapMode::Repeat:        return GL_REPEAT;
        case WrapMode::ClampToEdge:   return GL_CLAMP_TO_EDGE;
        case WrapMode::ClampToBorder: return GL_CLAMP_TO_BORDER;
        default:                      return GL_REPEAT;
      }
    }
  };
}