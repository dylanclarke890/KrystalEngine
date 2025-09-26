#pragma once

#include "Krystal.Gfx.OpenGL/Shaders/Shader.hpp"
#include "Krystal.Gfx/Common.hpp"
#include "Krystal.Gfx/Registries/IShaderRegistry.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.Gfx/ShaderPreprocessorConfig.hpp"
#include "Krystal.IO/Streams/NativeFileStream.hpp"
#include "Krystal.IO/Streams/StreamUtils.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys::Gfx::OpenGL
{
  class ShaderRegistry final : public IShaderRegistry
  {
    NO_COPY_MOVE(ShaderRegistry)

    using ShaderManager = ResourceManager<Shader, ShaderHandle>;
    using ShaderHandleCache = ResourceHandleCache<string, ShaderHandle>;

  private:
    ShaderManager _shaders;
    ShaderHandleCache _cache;

  public:
    ShaderRegistry() = default;
    ~ShaderRegistry() noexcept override = default;

    void Startup() noexcept override
    {
    }

    void Shutdown() noexcept override
    {
    }

    NO_DISCARD ShaderHandle Load(const IO::Path &vertex, const IO::Path &fragment) noexcept override
    {
      auto key = vertex.ToString() + "|" + fragment.ToString();
      if (auto cached = _cache.Get(key); cached.IsValid())
      {
        return cached;
      }

      Shader shader {ReadFile(vertex), ReadFile(fragment)};
      return AddShader(key, std::move(shader));
    }

    NO_DISCARD ShaderHandle Load(const IO::Path &vertex, const IO::Path &geometry,
                                 const IO::Path &fragment) noexcept override
    {
      auto key = vertex.ToString() + "|" + geometry.ToString() + "|" + fragment.ToString();
      if (auto cached = _cache.Get(key); cached.IsValid())
      {
        return cached;
      }

      Shader shader {ReadFile(vertex), ReadFile(geometry), ReadFile(fragment)};
      return AddShader(key, std::move(shader));
    }

    NO_DISCARD ShaderHandle Load(const IO::Path &vertex, const IO::Path &fragment,
                                 const ShaderPreprocessorConfig &config) noexcept
    {
      string defines {};
      for (const auto &[key, value] : config.Defines)
      {
        defines += std::format("#define {} {}\n", key, value);
      }

      auto key = vertex.ToString() + "|" + fragment.ToString() + "|" + defines;
      if (auto cached = _cache.Get(key); cached.IsValid())
      {
        return cached;
      }

      string vertexSource = defines + ReadFile(vertex);
      string fragmentSource = defines + ReadFile(fragment);
      Shader shader {std::move(vertexSource), std::move(fragmentSource)};

      return AddShader(key, std::move(shader));
    }

    NO_DISCARD ShaderHandle LoadTextShader(const IO::Path &vertex, const IO::Path &fragment,
                                           const TextShaderDesc &desc) noexcept override
    {
      ShaderPreprocessorConfig cfg {};

      switch (desc.FontType)
      {
        case FontType::Bitmap: cfg.Define("FONT_BITMAP", "1"); break;
        case FontType::SDF:    cfg.Define("FONT_SDF", "1"); break;
        case FontType::MSDF:   cfg.Define("FONT_MSDF", "1"); break;
        case FontType::MTSDF:  cfg.Define("FONT_MTSDF", "1"); break;
      }

      cfg.Define("ENABLE_SHADOW", desc.EnableShadow ? "1" : "0");
      cfg.Define("ENABLE_OUTLINE", desc.EnableOutline ? "1" : "0");
      cfg.Define("ENABLE_GLOW", desc.EnableGlow ? "1" : "0");

      return Load(vertex, fragment, cfg);
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