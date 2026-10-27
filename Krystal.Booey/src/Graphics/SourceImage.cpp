#include "Krystal.Booey/Graphics/SourceImage.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"

namespace krys::boo
{
  SourceImage::SourceImage(ImageVariant &&imageVariant) noexcept : _imageVariant(krys::move(imageVariant))
  {
  }

  SourceImage::SourceImage(const SourceImage &) noexcept = default;
  SourceImage::SourceImage(SourceImage &&) noexcept = default;
  SourceImage &SourceImage::operator=(const SourceImage &) noexcept = default;
  SourceImage &SourceImage::operator=(SourceImage &&) noexcept = default;
  SourceImage::~SourceImage() noexcept = default;

  bool SourceImage::operator==(const SourceImage &other) const noexcept
  {
    return ImageIdentifier() == other.ImageIdentifier();
  }

  namespace
  {
    NativeImage *NativeImageOf(const SourceImage::ImageVariant &imageVariant) noexcept
    {
      if (auto *nativeImage = std::get_if<Ref<NativeImage>>(&imageVariant))
      {
        return nativeImage->get();
      }
      return nullptr;
    }

    ImageBuffer *ImageBufferOf(const SourceImage::ImageVariant &imageVariant) noexcept
    {
      if (auto *imageBuffer = std::get_if<Ref<ImageBuffer>>(&imageVariant))
      {
        return imageBuffer->get();
      }
      return nullptr;
    }
  }

  NativeImage *SourceImage::NativeImageIfExists() const noexcept
  {
    return NativeImageOf(_imageVariant);
  }

  NativeImage *SourceImage::GetNativeImage() const noexcept
  {
    // FIXME: An image buffer's native image (createNativeImageReference), once image buffers have bitmap
    // backends; they record display lists.
    return NativeImageIfExists();
  }

  ImageBuffer *SourceImage::ImageBufferIfExists() const noexcept
  {
    return ImageBufferOf(_imageVariant);
  }

  ImageBuffer *SourceImage::GetImageBuffer() const noexcept
  {
    if (!std::holds_alternative<Ref<NativeImage>>(_imageVariant))
    {
      return ImageBufferIfExists();
    }

    if (!_transformedImageVariant)
    {
      const auto &nativeImage = std::get<Ref<NativeImage>>(_imageVariant);

      auto rect = FloatRect {{}, FloatSize(nativeImage->Size())};
      auto imageBuffer = ImageBuffer::Create(FloatSize(nativeImage->Size()), 1);
      if (!imageBuffer)
      {
        return nullptr;
      }

      imageBuffer->Context().DrawNativeImage(*nativeImage, rect, rect);
      Ref<ImageBuffer> buffer = krys::move(imageBuffer);
      _transformedImageVariant = ImageVariant {krys::move(buffer)};
    }

    krys_debug_assert(_transformedImageVariant);
    return ImageBufferOf(*_transformedImageVariant);
  }

  RenderingResourceIdentifier SourceImage::ImageIdentifier() const noexcept
  {
    return std::visit(CreateVisitor([](const Ref<NativeImage> &nativeImage)
                                    { return nativeImage->GetRenderingResourceIdentifier(); },
                                    [](const Ref<ImageBuffer> &imageBuffer)
                                    { return imageBuffer->GetRenderingResourceIdentifier(); }),
                      _imageVariant);
  }

  IntSize SourceImage::Size() const noexcept
  {
    return std::visit(CreateVisitor([](const Ref<NativeImage> &nativeImage) { return nativeImage->Size(); },
                                    [](const Ref<ImageBuffer> &imageBuffer)
                                    { return imageBuffer->BackendSize(); }),
                      _imageVariant);
  }
}
