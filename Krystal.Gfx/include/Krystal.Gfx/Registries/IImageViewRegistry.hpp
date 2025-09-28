#pragma once

#include "Krystal.Gfx/Common.hpp"
#include "Krystal.Gfx/Handle.hpp"

namespace Krys::Gfx
{
  struct ImageViewDesc
  {
    ImageHandle Image {};
    ImageType Target {ImageType::Image2D};
    ImageFormat Format {ImageFormat::R8};
    SubResourceRange SubResourceRange {};
  };

  class IImageViewRegistry
  {
  public:
    virtual ~IImageViewRegistry() noexcept = default;

    virtual void Startup() noexcept = 0;

    virtual void Shutdown() noexcept = 0;

    NO_DISCARD virtual ImageViewHandle Create(const ImageViewDesc &desc) noexcept = 0;

    virtual void Unload(ImageViewHandle handle) noexcept = 0;
  };
}