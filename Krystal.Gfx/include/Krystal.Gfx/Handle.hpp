#pragma once

#include "Krystal.Gfx.Lib/Handle.hpp"

namespace Krys::Gfx
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

  struct FontHandle : Handle<FontHandle>
  {
  };

  struct FontFamilyHandle : Handle<FontFamilyHandle>
  {
  };

  struct RenderTargetHandle : Handle<RenderTargetHandle>
  {
  };
}