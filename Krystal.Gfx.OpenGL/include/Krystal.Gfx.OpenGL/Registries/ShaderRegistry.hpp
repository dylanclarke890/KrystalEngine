#pragma once

#include "Krystal.Debug/ScopedProfiler.hpp"
#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.Gfx.OpenGL/Debug.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Shader.hpp"
#include "Krystal.Gfx/Registries/IShaderRegistry.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.IO/Streams/StreamUtils.hpp"
#include "Krystal.IO/VirtualFileSystem.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Lib/String/String.hpp"

namespace Krys::Gfx::OpenGL
{
  class ShaderRegistry final : public IShaderRegistry
  {
    NO_COPY_MOVE(ShaderRegistry)

    using ShaderManager = ResourceManager<Shader, ShaderHandle>;
    using ShaderHandleCache = ResourceHandleCache<string, ShaderHandle>;

    const IO::Path BaseDirectory {"/shaders/"};

  private:
    IO::VirtualFileSystem &_vfs;
    ShaderManager _shaders;
    ShaderHandleCache _cache;
    Map<BuiltinShader, ShaderHandle> _builtins;

  public:
    ShaderRegistry(IO::VirtualFileSystem &vfs) noexcept : _vfs(vfs), _shaders(), _cache()
    {
    }

    ~ShaderRegistry() noexcept override = default;

    void Startup() override
    {
    }

    void Shutdown() noexcept override
    {
    }

    KRYS_NODISCARD ShaderHandle Load(const IO::Path &vertex, const IO::Path &fragment) noexcept override
    {
      auto key = vertex.ToString() + "|" + fragment.ToString();
      if (auto cached = _cache.Get(key); cached.IsValid())
      {
        return cached;
      }

      Shader shader {ReadFile(vertex), ReadFile(fragment)};
      return AddShader(key, std::move(shader));
    }

    KRYS_NODISCARD ShaderHandle Load(const IO::Path &vertex, const IO::Path &geometry,
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

    KRYS_NODISCARD ShaderHandle Load(const IO::Path &vertex, const IO::Path &fragment,
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

      Shader shader {AddDefines(ReadFile(vertex), defines), AddDefines(ReadFile(fragment), defines)};
      return AddShader(key, std::move(shader));
    }

    KRYS_NODISCARD ShaderHandle GetBuiltin(BuiltinShader builtin) noexcept override
    {
      // auto profiler = Krys::Debug::ScopedProfiler("GetBuiltin");

      if (auto it = _builtins.find(builtin); it != _builtins.end())
      {
        return it->second;
      }

      string name;
      auto shader = [&]()
      {
        switch (builtin)
        {
          case BuiltinShader::Shape2D_Colour:
            name = "Builtin_Shape2DColour";
            return GetBuiltin_Shape2DColour();
          case BuiltinShader::Shape2D_Texture:
            name = "Builtin_Shape2DTexture";
            return GetBuiltin_Shape2DTexture();
          case BuiltinShader::PostProcess_Passthrough:
            name = "Builtin_PostProcessPassthrough";
            return GetBuiltin_PostProcessPassthrough();
          case BuiltinShader::Font_Bitmap:
            name = "Builtin_FontBitmap";
            return GetBuiltin_Text(FontType::Bitmap, false);
          case BuiltinShader::Font_SDF:
            name = "Builtin_FontSDF";
            return GetBuiltin_Text(FontType::SDF, false);
          case BuiltinShader::Font_SDF_Outline:
            name = "Builtin_FontSDFOutline";
            return GetBuiltin_Text(FontType::SDF, true);
          case BuiltinShader::Font_MSDF:
            name = "Builtin_FontMSDF";
            return GetBuiltin_Text(FontType::MSDF, false);
          case BuiltinShader::Font_MSDF_Outline:
            name = "Builtin_FontMSDFOutline";
            return GetBuiltin_Text(FontType::MSDF, true);
          case BuiltinShader::Font_MTSDF:
            name = "Builtin_FontMTSDF";
            return GetBuiltin_Text(FontType::MTSDF, false);
          case BuiltinShader::Font_MTSDF_Outline:
            name = "Builtin_FontMTSDFOutline";
            return GetBuiltin_Text(FontType::MTSDF, true);
        }
        std::unreachable();
      }();

      _builtins[builtin] = shader;
      Debug::SetName(_shaders.Get(shader), name);

      return shader;
    }

    bool Unload(ShaderHandle handle) noexcept override
    {
      assert(handle.IsValid() && "Handle is invalid.");
      assert(_shaders.TryGet(handle) != nullptr && "Shader not found in resource manager.");

      if (_cache.Remove(handle))
      {
        return _shaders.Remove(handle);
      }
      return false;
    }

    KRYS_NODISCARD Shader &Get(ShaderHandle handle) noexcept
    {
      return _shaders.Get(handle);
    }

  private:
    KRYS_NODISCARD ShaderHandle AddShader(const string &key, Shader &&shader) noexcept
    {
      auto handle = _shaders.Add(std::move(shader));
      _cache.Add(key, handle);
      return handle;
    }

    KRYS_NODISCARD string ReadFile(const IO::Path &filepath) noexcept
    {
      Unique<IO::IStreamReader> reader = _vfs.GetReader(BaseDirectory / filepath, IO::ReadFlags::None);
      assert(reader != nullptr && "Failed to create stream reader for shader file.");

      auto result = IO::StreamUtils::ReadAllText(*reader);
      assert(result.has_value() && "Failed to read shader file.");

      return result.value();
    }

    KRYS_NODISCARD static string AddDefines(const string &source, const string &defines) noexcept
    {
      const string lineDirective = "#line 1\n";
      string versionLine = source.substr(0, source.find('\n') + 1);
      return versionLine + defines + lineDirective + source.substr(versionLine.size());
    }

    KRYS_NODISCARD ShaderHandle GetBuiltin_Text(FontType fontType, bool outlined)
    {
      ShaderPreprocessorConfig cfg {};
      switch (fontType)
      {
        case FontType::Bitmap: cfg.Define("FONT_BITMAP", "1"); break;
        case FontType::SDF:    cfg.Define("FONT_SDF", "1"); break;
        case FontType::MSDF:   cfg.Define("FONT_MSDF", "1"); break;
        case FontType::MTSDF:  cfg.Define("FONT_MTSDF", "1"); break;
      }

      if (outlined)
      {
        cfg.Define("FEATURE_OUTLINE", "1");
      }

      const auto vertexShader = IO::Path("text-shader.vert");
      const auto fragmentShader = IO::Path("text-shader.frag");
      return Load(vertexShader, fragmentShader, cfg);
    }

    KRYS_NODISCARD ShaderHandle GetBuiltin_Shape2DColour()
    {
      const auto vertexShader = IO::Path("2d-shape.vert");
      const auto fragmentShader = IO::Path("2d-shape-colour.frag");
      return Load(vertexShader, fragmentShader);
    }

    KRYS_NODISCARD ShaderHandle GetBuiltin_Shape2DTexture()
    {
      const auto vertexShader = IO::Path("2d-shape.vert");
      const auto fragmentShader = IO::Path("2d-shape-texture.frag");
      return Load(vertexShader, fragmentShader);
    }

    KRYS_NODISCARD ShaderHandle GetBuiltin_PostProcessPassthrough()
    {
      const auto vertexShader = IO::Path("post-process.vert");
      const auto fragmentShader = IO::Path("post-process-passthrough.frag");
      return Load(vertexShader, fragmentShader);
    }
  };
}