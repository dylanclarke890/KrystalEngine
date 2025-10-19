#pragma once

#include "Krystal.Gfx.OpenGL/Registries/TextureRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Material.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Registries/IMaterialRegistry.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Gfx.Lib/ResourceManager.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys::Gfx::OpenGL
{
  class MaterialRegistry final : public IMaterialRegistry
  {
    NO_COPY_MOVE(MaterialRegistry)

    using MaterialManager = ResourceManager<Material, MaterialHandle>;
    using MaterialCache = ResourceHandleCache<string, MaterialHandle>;

  private:
    MaterialManager _materials;
    TextureRegistry &_textures;
    MaterialCache _cache;

  public:
    MaterialRegistry(TextureRegistry &textures) noexcept : _textures(textures)
    {
    }

    ~MaterialRegistry() noexcept
    {
    }

    void Startup() noexcept override
    {
    }

    void Shutdown() noexcept override
    {
    }

    NO_DISCARD MaterialHandle Create(const string &name, ShaderHandle shader,
                                     const PBRMaterialDesc &desc) noexcept override
    {
      if (auto cached = _cache.Get(name); cached.IsValid())
      {
        return cached;
      }

      List<MaterialParameter> params;
      using MPT = MaterialParameterType;

      params.push_back({"u_AlbedoFactor", MPT::Vec4, Colour::ToVec4(desc.AlbedoFactor)});
      params.push_back({"u_MetallicFactor", MPT::Float, desc.MetallicFactor});
      params.push_back({"u_RoughnessFactor", MPT::Float, desc.RoughnessFactor});
      params.push_back({"u_AmbientOcclusionFactor", MPT::Float, desc.AmbientOcclusionFactor});
      params.push_back({"u_NormalScale", MPT::Float, desc.NormalScale});
      params.push_back({"u_EmissiveFactor", MPT::Vec4, Colour::ToVec4(desc.EmissiveFactor)});
      params.push_back({"u_AlbedoMap", MPT::Texture, desc.AlbedoMap});
      params.push_back({"u_MetallicMap", MPT::Texture, desc.MetallicMap});
      params.push_back({"u_RoughnessMap", MPT::Texture, desc.RoughnessMap});
      params.push_back({"u_AmbientOcclusionMap", MPT::Texture, desc.AmbientOcclusionMap});
      params.push_back({"u_NormalMap", MPT::Texture, desc.NormalMap});
      params.push_back({"u_EmissiveMap", MPT::Texture, desc.EmissiveMap});

      Material material {name, shader, params};
      auto handle = _materials.Add(std::move(material));
      _cache.Add(name, handle);

      return handle;
    }

    NO_DISCARD MaterialHandle LoadPBRMaterial(const string &name, ShaderHandle shader) noexcept
    {
      if (auto cached = _cache.Get(name); cached.IsValid())
      {
        return cached;
      }

      using namespace IO;
      Path base = Path("/textures/pbr") / Path(name);

      PBRMaterialDesc desc;
      desc.AlbedoMap = _textures.Load(base / Path("albedo.png"));
      desc.NormalMap = _textures.Load(base / Path("normal.png"));
      desc.MetallicMap = _textures.Load(base / Path("metallic.png"));
      desc.RoughnessMap = _textures.Load(base / Path("roughness.png"));
      desc.AmbientOcclusionMap = _textures.Load(base / Path("ao.png"));

      return Create(name, shader, desc);
    }

    NO_DISCARD Material &Get(MaterialHandle handle) noexcept
    {
      return _materials.Get(handle);
    }

    void Unload(MaterialHandle handle) noexcept override
    {
      assert(handle.IsValid() && "Invalid handle.");
      assert(_materials.TryGet(handle) != nullptr && "Material not found in manager.");

      if (_cache.Remove(handle))
      {
        Material &material = _materials.Get(handle);
        for (const auto &param : material.Parameters)
        {
          if (param.Type == MaterialParameterType::Texture)
          {
            TextureHandle textureHandle = std::get<TextureHandle>(param.Value);
            if (textureHandle.IsValid())
            {
              _textures.Unload(textureHandle);
            }
          }
        }
        _materials.Remove(handle);
      }
    }
  };
}