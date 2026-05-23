#pragma once

#include "Krystal.Gfx.OpenGL/Mappers/Enums/ImageType.hpp"
#include "Krystal.Gfx.OpenGL/Mappers/Enums/PixelFormat.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Image.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Registries/IImageRegistry.hpp"
#include "Krystal.Gfx/ResourceManager.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys::Gfx::OpenGL
{
  class ImageRegistry : public IImageRegistry
  {
    using ImageManager = ResourceManager<Image, ImageHandle>;

  private:
    ImageManager _images;

  public:
    ImageRegistry() = default;

    ~ImageRegistry() noexcept override = default;

    void Startup() override
    {
    }

    void Shutdown() noexcept override
    {
    }

    KRYS_NODISCARD ImageHandle Create(const ImageDesc &desc)
    {
      GLenum target = MapImageType(desc.Type);
      GLenum internalFormat = MapPixelFormat(desc.Format);
      Image image {
        target, internalFormat, desc.Width, desc.Height, desc.Depth, desc.MipLevels, desc.ArrayLayers,
      };
      return _images.Add(std::move(image));
    }

    bool Destroy(ImageHandle image) noexcept override
    {
      return _images.Remove(image);
    }

    KRYS_NODISCARD Image &Get(ImageHandle handle)
    {
      return _images.Get(handle);
    }

    KRYS_NODISCARD Image *TryGet(ImageHandle handle) noexcept
    {
      return _images.TryGet(handle);
    }
  };
}