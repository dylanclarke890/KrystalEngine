#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Image.hpp"

namespace krys::Gfx
{
  class IImageRegistry
  {
    KRYS_NON_COPY_MOVABLE(IImageRegistry);

  protected:
    IImageRegistry() noexcept = default;

  public:
    virtual ~IImageRegistry() noexcept = default;

    virtual void Startup() = 0;

    virtual void Shutdown() noexcept = 0;

    KRYS_NODISCARD virtual ImageHandle Create(const ImageDesc &desc) = 0;

    virtual bool Destroy(ImageHandle handle) noexcept = 0;
  };
}