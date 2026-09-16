#pragma once

#include "Krystal.Core/Utils/Handle.hpp"

namespace krys::Gfx
{
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

  struct BufferHandle : Handle<BufferHandle>
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

  struct FontFamilyHandle : Handle<FontFamilyHandle>
  {
  };

  struct FontHandle : Handle<FontHandle>
  {
  };

  struct RenderTargetHandle : Handle<RenderTargetHandle>
  {
  };
}