#pragma once

#include "Krystal.Gfx.Lib/ResourceManager.hpp"
#include "Krystal.Gfx.OpenGL/Resources/Image.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Registries/IImageRegistry.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Macros.hpp"

namespace Krys::Gfx::OpenGL
{
  class ImageRegistry : public IImageRegistry
  {
    NO_COPY_MOVE(ImageRegistry)

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

    NO_DISCARD ImageHandle Create(const ImageDesc &desc)
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

    NO_DISCARD Image &Get(ImageHandle handle)
    {
      return _images.Get(handle);
    }

    NO_DISCARD Image *TryGet(ImageHandle handle) noexcept
    {
      return _images.TryGet(handle);
    }
  };
}