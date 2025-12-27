#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Resources/Image.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Macros.hpp"

namespace Krys::Gfx
{
  class IImageRegistry
  {
    NO_COPY_MOVE(IImageRegistry)

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