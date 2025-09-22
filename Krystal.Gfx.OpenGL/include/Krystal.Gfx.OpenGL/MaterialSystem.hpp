#pragma once

#include "Krystal.Gfx.OpenGL/Material.hpp"
#include "Krystal.Gfx.OpenGL/TextureSystem.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/IMaterialSystem.hpp"
#include "Krystal.Gfx/ResourceHandleCache.hpp"
#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Map.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys::Gfx::OpenGL
{

  class MaterialSystem final : public IMaterialSystem
  {
    NO_COPY_MOVE(MaterialSystem)

    using MaterialManager = ResourceManager<Material, MaterialHandle>;
    using MaterialCache = ResourceHandleCache<string, MaterialHandle>;

  private:
    MaterialManager _materials;
    MaterialCache _cache;

  public:
    MaterialSystem() = default;
    ~MaterialSystem() noexcept = default;

    NO_DISCARD MaterialHandle Create(const string &name, ShaderHandle shader,
                                     const PBRMaterialDesc &desc) noexcept override
    {
      auto existing = _cache.Get(name);
      if (existing.IsValid())
      {
        return existing;
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

    NO_DISCARD MaterialHandle LoadPBRMaterial(const string &name, ShaderHandle shader,
                                              TextureSystem &textureSystem) noexcept
    {
      using namespace IO;
      Path base = Path("data/assets/pbr/") / Path(name);

      PBRMaterialDesc desc;
      desc.AlbedoMap = textureSystem.Load(base / Path("albedo.png"));
      desc.NormalMap = textureSystem.Load(base / Path("normal.png"));
      desc.MetallicMap = textureSystem.Load(base / Path("metallic.png"));
      desc.RoughnessMap = textureSystem.Load(base / Path("roughness.png"));
      desc.AmbientOcclusionMap = textureSystem.Load(base / Path("ao.png"));

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
        _materials.Remove(handle);
      }
    }
  };
}