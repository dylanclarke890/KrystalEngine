#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Attributes.hpp"

namespace Krys::Gfx
{
  struct TextureDesc
  {
    bool IsSRGB {false};
    bool GenerateMipmaps {true};
    uint32 MaxMipLevels {0u};
  };

  class ITextureSystem
  {
  public:
    virtual ~ITextureSystem() = default;

    NO_DISCARD virtual TextureHandle Load(const IO::Path &path, const TextureDesc &desc) noexcept = 0;

    NO_DISCARD virtual TextureHandle LoadCubemap(const IO::Path &left, const IO::Path &right,
                                                 const IO::Path &top, const IO::Path &bottom,
                                                 const IO::Path &front, const IO::Path &back,
                                                 const TextureDesc &desc = {}) noexcept = 0;

    virtual void Unload(TextureHandle handle) noexcept = 0;
  };
}