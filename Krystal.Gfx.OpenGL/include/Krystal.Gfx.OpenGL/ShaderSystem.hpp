#pragma once

#include "Krystal.Gfx.OpenGL/Shader.hpp"
#include "Krystal.Gfx/IShaderSystem.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include "Krystal.IO/Streams/StreamUtils.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys::Gfx::OpenGL
{
  class ShaderSystem final : public IShaderSystem
  {
    NO_COPY_MOVE(ShaderSystem)

    using ShaderManager = ResourceManager<Shader, ShaderHandle>;
    using ShaderHandleCache = ResourceHandleCache<string, ShaderHandle>;

    struct ShaderResource
    {
      ShaderHandle Handle;
      uint16 ReferenceCount {1u};
    };

  private:
    ShaderManager _shaders;
    ShaderHandleCache _cache;

  public:
    ShaderSystem() = default;
    ~ShaderSystem() noexcept override = default;

    NO_DISCARD ShaderHandle Load(const IO::Path &vertex, const IO::Path &fragment) noexcept override
    {
      auto key = vertex.ToString() + "|" + fragment.ToString();
      auto existing = _cache.Get(key);
      if (existing.IsValid())
      {
        return existing;
      }

      Shader shader {ReadFile(vertex), ReadFile(fragment)};
      return AddShader(key, std::move(shader));
    }

    NO_DISCARD ShaderHandle Load(const IO::Path &vertex, const IO::Path &geometry,
                                 const IO::Path &fragment) noexcept override
    {
      auto key = vertex.ToString() + "|" + geometry.ToString() + "|" + fragment.ToString();
      auto existing = _cache.Get(key);
      if (existing.IsValid())
      {
        return existing;
      }

      Shader shader {ReadFile(vertex), ReadFile(geometry), ReadFile(fragment)};
      return AddShader(key, std::move(shader));
    }

    void Unload(ShaderHandle handle) noexcept override
    {
      assert(handle.IsValid() && "Handle is invalid.");
      assert(_shaders.TryGet(handle) != nullptr && "Shader not found in resource manager.");

      if (_cache.Remove(handle))
      {
        _shaders.Remove(handle);
      }
    }

    NO_DISCARD Shader &Get(ShaderHandle handle) noexcept
    {
      return _shaders.Get(handle);
    }

  private:
    string ReadFile(const IO::Path &filepath) noexcept
    {
      IO::NativeFileReader reader {filepath};
      auto result = IO::StreamUtils::ReadAllText(reader);
      assert(result.has_value() && "Failed to read shader file.");
      return result.value();
    }

    ShaderHandle AddShader(const string &key, Shader &&shader) noexcept
    {
      auto handle = _shaders.Add(std::move(shader));
      _cache.Add(key, handle);
      return handle;
    }
  };
}