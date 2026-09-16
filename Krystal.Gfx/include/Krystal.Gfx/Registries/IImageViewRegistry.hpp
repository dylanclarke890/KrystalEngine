#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/ImageView.hpp"

namespace krys::Gfx
{
  class IImageViewRegistry
  {
    KRYS_NON_COPY_MOVABLE(IImageViewRegistry);

  protected:
    IImageViewRegistry() noexcept = default;

  public:
    virtual ~IImageViewRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    KRYS_NODISCARD virtual ImageViewHandle Create(const ImageViewDesc &desc) noexcept = 0;

    virtual bool Destroy(ImageViewHandle handle) noexcept = 0;
  };
}