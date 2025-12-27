#pragma once

#include "Krystal.Gfx.Lib/ResourceManager.hpp"
#include "Krystal.Gfx.OpenGL/Registries/ImageRegistry.hpp"
#include "Krystal.Gfx.OpenGL/Resources/ImageView.hpp"
#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Gfx/Registries/IImageViewRegistry.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Macros.hpp"

namespace Krys::Gfx::OpenGL
{
  class ImageViewRegistry : public IImageViewRegistry
  {
    NO_COPY_MOVE(ImageViewRegistry)

    using ImageViewManager = ResourceManager<ImageView, ImageViewHandle>;

  private:
    ImageRegistry &_images;
    ImageViewManager _imageViews;

  public:
    ImageViewRegistry(ImageRegistry &images) noexcept : _images(images)
    {
    }

    ~ImageViewRegistry() noexcept override = default;

    void Startup() noexcept override
    {
    }

    void Shutdown() noexcept override
    {
    }

    NO_DISCARD ImageViewHandle Create(const ImageViewDesc &desc) noexcept
    {
      assert(desc.Image.IsValid() && "Invalid image handle.");
      auto *image = _images.TryGet(desc.Image);
      if (image == nullptr) [[unlikely]]
      {
        assert(false && "Image handle does not reference a valid image.");
        return ImageViewHandle {};
      }

      GLenum target = MapImageType(desc.Target);
      GLenum internalFormat = MapPixelFormat(desc.Format);
      ImageView imageView {desc.Image, image->Id(), target, internalFormat, desc.SubResourceRange};
      return _imageViews.Add(std::move(imageView));
    }

    bool Destroy(ImageViewHandle handle) noexcept override
    {
      return _imageViews.Remove(handle);
    }

    NO_DISCARD ImageView &Get(ImageViewHandle handle)
    {
      return _imageViews.Get(handle);
    }

    NO_DISCARD ImageView *TryGet(ImageViewHandle handle)
    {
      return _imageViews.TryGet(handle);
    }
  };
}