#pragma once

#include "Krystal.Gfx/Common.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Span.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::Gfx
{
  struct ImageDesc
  {
    ImageType Type;
    ImageFormat Format;
    uint32 Width;
    uint32 Height;
    uint32 Depth;
    uint32 MipLevels;
    uint32 ArrayLayers;
  };

  class IImageRegistry
  {
    NO_COPY_MOVE(IImageRegistry)

  protected:
    IImageRegistry() noexcept = default;

  public:
    virtual ~IImageRegistry() noexcept = default;

    virtual void Startup() noexcept = 0;

    virtual void Shutdown() noexcept = 0;

    NO_DISCARD virtual ImageHandle Create(const ImageDesc &desc) noexcept = 0;

    virtual void Unload(ImageHandle handle) noexcept = 0;
  };
}