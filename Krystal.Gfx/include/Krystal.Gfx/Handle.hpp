#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  template <typename Derived>
  struct Handle
  {
    uint32 Id {0u};

    NO_DISCARD bool IsValid() const noexcept
    {
      return Id != 0u;
    }
  };

  struct ImageHandle : Handle<ImageHandle>
  {
  };

  struct ImageViewHandle : Handle<ImageViewHandle>
  {
  };

  struct SamplerHandle : Handle<SamplerHandle>
  {
  };

  struct ShaderHandle : Handle<ShaderHandle>
  {
  };

  struct MeshHandle : Handle<MeshHandle>
  {
  };

  struct TextureHandle : Handle<TextureHandle>
  {
  };

  struct MaterialHandle : Handle<MaterialHandle>
  {
  };

  struct FontHandle : Handle<MaterialHandle>
  {
  };
}