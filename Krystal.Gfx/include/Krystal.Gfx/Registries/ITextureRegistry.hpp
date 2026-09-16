#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/IO/Path.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Gfx/Handle.hpp"

namespace krys::Gfx
{
  struct TextureDesc
  {
    bool IsSRGB {false};
    bool GenerateMipmaps {true};
    uint32 MaxMipLevels {0u};
  };

  class ITextureRegistry
  {
    KRYS_NON_COPY_MOVABLE(ITextureRegistry);

  protected:
    ITextureRegistry() noexcept = default;

  public:
    virtual ~ITextureRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    KRYS_NODISCARD virtual TextureHandle Create(ImageViewHandle imageView,
                                                SamplerHandle sampler) noexcept = 0;

    KRYS_NODISCARD virtual TextureHandle Load(const io::Path &path, const TextureDesc &desc) noexcept = 0;

    KRYS_NODISCARD virtual TextureHandle LoadCubemap(const io::Path &left, const io::Path &right,
                                                     const io::Path &top, const io::Path &bottom,
                                                     const io::Path &front, const io::Path &back,
                                                     const TextureDesc &desc = {}) noexcept = 0;

    virtual bool Unload(TextureHandle handle) noexcept = 0;
  };
}