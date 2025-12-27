#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.IO/Path.hpp"
#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Macros.hpp"

namespace Krys::Gfx
{
  struct TextureDesc
  {
    bool IsSRGB {false};
    bool GenerateMipmaps {true};
    uint32 MaxMipLevels {0u};
  };

  class ITextureRegistry
  {
    NO_COPY_MOVE(ITextureRegistry)

  protected:
    ITextureRegistry() noexcept = default;

  public:
    virtual ~ITextureRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    KRYS_NODISCARD virtual TextureHandle Create(ImageViewHandle imageView, SamplerHandle sampler) noexcept = 0;

    KRYS_NODISCARD virtual TextureHandle Load(const IO::Path &path, const TextureDesc &desc) noexcept = 0;

    KRYS_NODISCARD virtual TextureHandle LoadCubemap(const IO::Path &left, const IO::Path &right,
                                                 const IO::Path &top, const IO::Path &bottom,
                                                 const IO::Path &front, const IO::Path &back,
                                                 const TextureDesc &desc = {}) noexcept = 0;

    virtual bool Unload(TextureHandle handle) noexcept = 0;
  };
}