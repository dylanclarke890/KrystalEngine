#pragma once

#include "Krystal.Gfx.OpenGL/gl.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Sampler.hpp"
#include "Krystal.Gfx/Registries/ISamplerRegistry.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Gfx/ResourceManager.hpp"
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
    using SamplerCache = ResourceHandleCache<size_t, SamplerHandle>;

  private:
    SamplerManager _samplers;
    SamplerCache _cache;

  public:
    SamplerRegistry() = default;

    ~SamplerRegistry() noexcept override = default;

    void Startup() noexcept override
    {
    }

    void Shutdown() noexcept override
    {
    }

    NO_DISCARD SamplerHandle Create(const SamplerDesc &desc) noexcept override
    {
      GLenum min = MapFilterMode(desc.MinFilter);
      GLenum mag = MapFilterMode(desc.MagFilter);
      assert(mag != GL_NEAREST_MIPMAP_NEAREST && mag != GL_LINEAR_MIPMAP_NEAREST
             && mag != GL_NEAREST_MIPMAP_LINEAR && mag != GL_LINEAR_MIPMAP_LINEAR
             && "Mag filter cannot be a mipmap filter.");
      GLenum wrapS = MapWrapMode(desc.WrapS);
      GLenum wrapT = MapWrapMode(desc.WrapT);
      GLenum wrapR = MapWrapMode(desc.WrapR);

      size_t key = HashUtils::HashCombine(min, mag, wrapS, wrapT, wrapR, desc.AnisotropicLevel);
      if (auto cached = _cache.Get(key); cached.IsValid())
      {
        return cached;
      }

      Sampler sampler {min, mag, wrapS, wrapT, wrapR, desc.AnisotropicLevel};
      SamplerHandle handle = _samplers.Add(std::move(sampler));
      _cache.Add(key, handle);

      return handle;
    }

    void Unload(SamplerHandle handle) noexcept override
    {
      assert(handle.IsValid() && "Invalid handle.");
      assert(_samplers.TryGet(handle) != nullptr && "Sampler not found in resource manager.");

      if (_cache.Remove(handle))
      {
        _samplers.Remove(handle);
      }
    }

  private:
    static GLenum MapFilterMode(FilterMode mode) noexcept
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

    static GLenum MapWrapMode(WrapMode mode) noexcept
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