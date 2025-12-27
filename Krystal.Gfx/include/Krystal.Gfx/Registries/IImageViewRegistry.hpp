#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/ImageView.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Gfx
{
  class IImageViewRegistry
  {
    NO_COPY_MOVE(IImageViewRegistry)

  protected:
    IImageViewRegistry() noexcept = default;

  public:
    virtual ~IImageViewRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    NO_DISCARD virtual ImageViewHandle Create(const ImageViewDesc &desc) noexcept = 0;

    virtual bool Destroy(ImageViewHandle handle) noexcept = 0;
  };
}